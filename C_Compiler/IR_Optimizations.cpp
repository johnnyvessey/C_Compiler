#include "IR_Optimizations.h"

void IR_Optimizations::RemoveNoOps(vector<shared_ptr<IR_Statement>>& irStatements)
{
	vector<shared_ptr<IR_Statement>> newStatements;
	newStatements.reserve(irStatements.size());

	for (const shared_ptr<IR_Statement>& statement : irStatements)
	{
		if (statement->GetType() != _IR_NOP)
		{
			newStatements.push_back(statement);
		}
	}
	newStatements.shrink_to_fit();
	irStatements = newStatements;
}


void IR_Optimizations::OptimizeJumps(vector<shared_ptr<IR_Statement>>& irStatements)
{
	if (optimizeJumps)
	{
		RemoveJumpToNextLine(irStatements);
		RemoveConsecutiveJumps(irStatements);
		RemoveUnusedLabels(irStatements);
	}
}

/*
	Remove a jump to a label that is on the next line
	i.e.:

	JLTE #1
	Label #1

	This is useful for simplifying the if/then chains and and/or conditional parsing.
*/
void IR_Optimizations::RemoveJumpToNextLine(vector<shared_ptr<IR_Statement>>& irStatements)
{

	for (int i = 0; i < int(irStatements.size()) - 1; ++i)
	{
		const shared_ptr<IR_Statement>& statement = irStatements.at(i);
		const shared_ptr<IR_Statement>& nextStatement = irStatements.at(i + 1);

		if (statement->GetType() == _IR_JUMP && nextStatement->GetType() == _IR_LABEL)
		{
			IR_Jump* jump = dynamic_cast<IR_Jump*>(statement.get());
			IR_Label* label = dynamic_cast<IR_Label*>(nextStatement.get());

			if (jump->labelIdx == label->label)
			{
				irStatements[i] = make_shared<IR_NOP>();
			}
		}
	}

	RemoveNoOps(irStatements);
}
void IR_Optimizations::RemoveConsecutiveJumps(vector<shared_ptr<IR_Statement>>& irStatements)
{


	RemoveNoOps(irStatements);
}

/*
	Remove labels that are never jumped to.
*/
void IR_Optimizations::RemoveUnusedLabels(vector<shared_ptr<IR_Statement>>& irStatements)
{

	unordered_set<int> labelSet;

	for (const shared_ptr<IR_Statement>& statement : irStatements)
	{
		if (statement->GetType() == _IR_JUMP)
		{
			IR_Jump* jump = dynamic_cast<IR_Jump*>(statement.get());

			labelSet.insert(jump->labelIdx);
		}
	}

	for (shared_ptr<IR_Statement>& statement : irStatements)
	{
		if (statement->GetType() == _IR_LABEL)
		{
			IR_Label* irLabel = dynamic_cast<IR_Label*>(statement.get());

			if (labelSet.find(irLabel->label) == labelSet.end())
			{
				statement = make_shared<IR_NOP>();
			}
		}
	}

	RemoveNoOps(irStatements);
}



bool matchFlags(FlagResults inputFlags, FlagResults jumpFlags)
{
	switch (jumpFlags)
	{
	case IR_EQUALS:
		return inputFlags == IR_EQUALS;
	case IR_NOT_EQUALS: 
		return inputFlags == IR_NOT_EQUALS || inputFlags == IR_GREATER || inputFlags == IR_LESS;
	case IR_LESS:
		return inputFlags == IR_LESS;
	case IR_LESS_EQUALS:
		return inputFlags == IR_LESS || inputFlags == IR_EQUALS || inputFlags == IR_LESS_EQUALS;
	case IR_GREATER:
		return inputFlags == IR_GREATER;
	case IR_GREATER_EQUALS:
		return inputFlags == IR_GREATER || inputFlags == IR_EQUALS || inputFlags == IR_GREATER_EQUALS;
	case IR_FLOAT_LESS:
		return inputFlags == IR_FLOAT_LESS;
	case IR_FLOAT_LESS_EQUALS:
		return inputFlags == IR_FLOAT_LESS || inputFlags == IR_EQUALS || inputFlags == IR_FLOAT_LESS_EQUALS;
	case IR_FLOAT_GREATER:
		return inputFlags == IR_FLOAT_GREATER;
	case IR_FLOAT_GREATER_EQUALS:
		return inputFlags == IR_FLOAT_GREATER || inputFlags == IR_EQUALS || inputFlags == IR_FLOAT_GREATER_EQUALS;
	}


	return false;
}

template <class T>
string computeConstantFolding(const T destVal, const T sourceVal, IR_AssignType type)
{
	switch (type) {
	case IR_ADD:
		return std::to_string(destVal + sourceVal);
	case IR_SUBTRACT:
		return std::to_string(destVal - sourceVal);
	case IR_MULTIPLY:
		return std::to_string(destVal * sourceVal);
	case IR_DIVIDE:
		return std::to_string(destVal / sourceVal);
	case IR_NEGATIVE:
		return std::to_string(-destVal);
	}


	return "";
}

/*
	Constant propagation method:
	- find all non-memory variables (ones that could be in registers)
	- have dictionary of variable to string which contains constant value
	- if IR_Assign statement is used, replace operands with constants
		-> if dest is constant, only replace if both dest and source are constants; in that case, do constant folding and assign
			constant to that value, and update constant dictionary
		-> if source is constant, just replace with literal and update dictionary
	- IMPORTANT: also keep track of flags
*/


bool isNonMemoryVariable(int varIndex, const unordered_map<int,int> nonRegisterVariables) {
	return nonRegisterVariables.find(varIndex) == nonRegisterVariables.end();
}

//IMPORTANT: be really careful about loops because backwards jumps could invalidate propagation
//figure out what to do
void IR_Optimizations::ConstantPropagationAndDCE(vector<shared_ptr<IR_Statement>>& irStatements, const unordered_map<int,int> nonRegisterVariables)
{
	unordered_map<int, string> constantDictionary;
	FlagResults currentConstantFlag = IR_NO_FLAGS;
	int loopLevel = 0;

	int labelToSearchFor = -1;

	for (shared_ptr<IR_Statement>& statement : irStatements)
	{
		bool saveFlags = false;

		if (labelToSearchFor >= 0) {
			if (statement->GetType() == _IR_LABEL) {
				IR_Label* label = dynamic_cast<IR_Label*>(statement.get());
				if (label->label == labelToSearchFor) {
					labelToSearchFor = -1;
				}
				else {
					statement = make_unique<IR_NOP>();
				}
			}
			else {
				statement = make_unique<IR_NOP>();
			}
		}
		//For now, don't do constant propogation inside loops...
		//in the future, iterate through the statements of the loop
		//	do constant propogation on copy of dict
		//	remove all elements that changed or were removed
		//  start over and do constant propogation with new dictionary
		else if (loopLevel > 0) {
			if (statement->GetType() == _IR_LOOP_START) ++loopLevel;
			else if (statement->GetType() == _IR_LOOP_END) {
				--loopLevel;
				if (loopLevel == 0) constantDictionary.clear();
			}
		}
		else if (statement->GetType() == _IR_ASSIGN)
		{
			IR_Assign* assign = dynamic_cast<IR_Assign*>(statement.get());
			

			//assign to constants first, and then do folding afterwards if possible
			if (constantDictionary.find(assign->dest.value.varIndex) != constantDictionary.end())
			{
				//only do this if non memory variable, need to pass in variable data...
				//assign->dest.value.valueType = IR_LITERAL;
				assign->dest.value.literalValue = constantDictionary.at(assign->dest.value.varIndex);
				
			}
			if (constantDictionary.find(assign->source.value.varIndex) != constantDictionary.end())
			{
				//only do this if non memory variable, need to pass in variable data...
				assign->source.value.valueType = IR_LITERAL;
				assign->source.value.literalValue = constantDictionary.at(assign->source.value.varIndex);
				
			}


			bool isDestConstant = assign->dest.value.valueType == IR_LITERAL || 
				constantDictionary.find(assign->dest.value.varIndex) != constantDictionary.end();
			bool isSourceConstant = assign->source.value.valueType == IR_LITERAL || 
				(assign->assignType == IR_FLAG_CONVERT && currentConstantFlag != IR_NO_FLAGS);

			if (isSourceConstant && isDestConstant 
				&& isNonMemoryVariable(assign->source.value.varIndex, nonRegisterVariables) && isNonMemoryVariable(assign->source.value.varIndex, nonRegisterVariables))
			{

				//do constant folding based on assign type
				if (assign->assignType == IR_FLAG_CONVERT)
				{
					//???
					bool match = matchFlags(currentConstantFlag, assign->source.value.flag);
					assign->assignType = IR_COPY;
					assign->source.value.valueType = IR_LITERAL;
					assign->source.value.literalValue = match ? "1" : "0";
					constantDictionary[assign->dest.value.varIndex] = match ? "1" : "0";
				}
				else {
					
					const string output = (assign->dest.value.type == IR_FLOAT) ? computeConstantFolding<double>(std::stod(assign->dest.value.literalValue),
						std::stod(assign->source.value.literalValue), assign->assignType) : computeConstantFolding<long long>(std::stoll(assign->dest.value.literalValue),
							std::stoll(assign->source.value.literalValue), assign->assignType);
					if (!output.empty()) {
						assign->source.value.literalValue = output;
						assign->source.value.valueType = IR_LITERAL;
						constantDictionary[assign->dest.value.varIndex] = output;
						assign->assignType = IR_COPY;
					}
				}
			}
			else if (isDestConstant)
			{
				//remove from constant dictionary???
				constantDictionary.erase(assign->dest.value.varIndex);
			}
			else if (isSourceConstant && isNonMemoryVariable(assign->dest.value.varIndex, nonRegisterVariables))
			{

				assign->source.value.valueType = IR_LITERAL;

				if (assign->assignType == IR_COPY || assign->assignType == IR_TYPE_CAST) //TODO: add in type-cast???
				{
					string literalValue = assign->source.value.literalValue;
					assign->source.value.literalValue = literalValue;
					constantDictionary[assign->dest.value.varIndex] = literalValue;
				}
				else if (assign->assignType == IR_FLAG_CONVERT && currentConstantFlag != IR_NO_FLAGS)
				{
					//assign based on flags
					bool match = matchFlags(currentConstantFlag, assign->source.value.flag);
					assign->assignType = IR_COPY;
					assign->source.value.valueType = IR_LITERAL;
					assign->source.value.literalValue = match ? "1" : "0";
					constantDictionary[assign->dest.value.varIndex] = match ? "1" : "0";
				}
			}
		}
		else if (statement->GetType() == _IR_COMPARE)
		{
			saveFlags = true;

			IR_Compare* compare = dynamic_cast<IR_Compare*>(statement.get());

			const bool foundOp1 = constantDictionary.find(compare->op1.value.varIndex) != constantDictionary.end();
			const bool foundOp2 = constantDictionary.find(compare->op2.value.varIndex) != constantDictionary.end();

			const bool op1Constant = compare->op1.value.valueType == IR_LITERAL || foundOp1;
			const bool op2Constant = compare->op2.value.valueType == IR_LITERAL || foundOp2;
			if (op1Constant && op2Constant)
			{
				const string op1ValString = foundOp1 ? constantDictionary.at(compare->op1.value.varIndex) : compare->op1.value.literalValue;
				const string op2ValString = foundOp2 ? constantDictionary.at(compare->op2.value.varIndex) : compare->op2.value.literalValue;
				bool isFloat = compare->op1.value.type == IR_FLOAT;


				//cast ints to floats as well (comparisons will be the same)
				float op1Val = std::stof(op1ValString);
				float op2Val = std::stof(op2ValString);

				if (op1Val == op2Val)
				{
					currentConstantFlag = IR_EQUALS;
				}
				else if (op1Val > op2Val)
				{
					currentConstantFlag = isFloat ? IR_FLOAT_GREATER : IR_GREATER;
				}
				else {
					currentConstantFlag = isFloat ? IR_FLOAT_LESS : IR_LESS;
				}

				statement = make_unique<IR_NOP>();
				
			}
			else {
				currentConstantFlag = IR_NO_FLAGS;
			}
		}
		else if (statement->GetType() == _IR_JUMP)
		{
			IR_Jump* jump = dynamic_cast<IR_Jump*>(statement.get());

			if (currentConstantFlag != IR_NO_FLAGS)
			{
				bool shouldJump = matchFlags(currentConstantFlag, jump->condition);
				jump->condition = shouldJump ? IR_ALWAYS : IR_NEVER;
			}

			//for now, clear the constant dictionary when there is branching
			if (jump->condition != IR_ALWAYS && jump->condition != IR_NEVER) {
				constantDictionary.clear();
			}

			if (jump->condition == IR_NEVER) {
				statement = make_unique<IR_NOP>();
			}
			else if (jump->condition == IR_ALWAYS) {
				labelToSearchFor = jump->labelIdx;
				statement = make_unique<IR_NOP>();
			}
			
		}
		else if (statement->GetType() == _IR_LOOP_START) {
			++loopLevel;
		}

		if (!saveFlags) {
			currentConstantFlag = IR_NO_FLAGS;
		}

	}

	//clean up after constant propogation and DCE
	OptimizeJumps(irStatements);
	RemoveNoOps(irStatements);
}

//remove all variables that are never the source of a statement or a return value or a function parameter or a compare (or other way which is
//"using" the variable
void IR_Optimizations::RemoveUnusedVariables(vector<shared_ptr<IR_Statement>>& irStatements, const unordered_map<int, int> nonRegisterVariables)
{

}
