#include "IR_Statement.h"
#include <sstream>

using std::stringstream;



OperandAsm IR_Statement::ConvertIrOperandToOperandAsm(IR_Operand& op, x64_State& state)
{
	OperandAsm opAsm;

	if (op.value.valueType == IR_LITERAL)
	{
		opAsm.type = ASM_INT_LITERAL;
		opAsm.literalIntValue = std::stoi(op.value.literalValue);
	}
	else if (op.globalFloatValue != 0)
	{
		opAsm.type = ASM_GLOBAL_MEMORY;
		opAsm.dereference = true;
		opAsm.name = std::to_string(op.globalFloatValue);
	}
	else if (op.value.specialVars == IR_RETURN_INT)
	{
		return OperandAsm::CreateRegisterOperand(state.AllocateRegister(op.value, RAX));
	}
	else if (op.value.specialVars == IR_RETURN_FLOAT)
	{
		return OperandAsm::CreateRegisterOperand(state.AllocateRegister(op.value, XMM0));
	}
	else if (op.value.specialVars == IR_RETURN_STACK)
	{
		return OperandAsm::CreateRegisterOperand(state.AllocateRegister(op.value, RDI));
	}
	//else if (state.registerAllocator.memoryVariableMapping.memoryOffsetMapping.find(op.value.varIndex) !=
	//	state.registerAllocator.memoryVariableMapping.memoryOffsetMapping.end())
	//{
	//	opAsm.reg = RegisterAsm(RSP);
	//	opAsm.baseOffset = state.registerAllocator.memoryVariableMapping.memoryOffsetMapping.at(op.value.varIndex);
	//	opAsm.baseOffset += op.baseOffset;
	//	
	//	opAsm.dereference = true;

	//	//TODO: figure out what to do if using memory address

	//}
	else {
		bool memoryVariable = state.registerAllocator.memoryVariableMapping.memoryOffsetMapping.find(op.value.varIndex) !=
			state.registerAllocator.memoryVariableMapping.memoryOffsetMapping.end();
		opAsm.dereference = op.dereference || memoryVariable;
		opAsm.baseOffset = op.baseOffset;

		if (memoryVariable)
		{
			opAsm.reg = RegisterAsm(RSP);
			opAsm.baseOffset += state.registerAllocator.memoryVariableMapping.memoryOffsetMapping.at(op.value.varIndex);
		}
		else {
			opAsm.reg = RegisterAsm(state.AllocateRegister(op.value));
		}

		if (op.memoryOffsetMultiplier != 0)
		{
			opAsm.useRegOffset = true;
			opAsm.regOffsetMultiplier = op.memoryOffsetMultiplier;
			opAsm.regOffset = RegisterAsm(state.AllocateRegister(op.memoryOffset));
		}

	}

	return opAsm;
}

string conditionToString(FlagResults condition)
{
	string cs = "";
	switch (condition)
	{
	case IR_EQUALS: cs = "E"; break;
	case IR_NOT_EQUALS: cs = "NE"; break;
	case IR_GREATER: cs = "G"; break;
	case IR_GREATER_EQUALS: cs = "GE"; break;
	case IR_LESS: cs = "L"; break;
	case IR_LESS_EQUALS: cs = "LE"; break;
	case IR_FLOAT_GREATER: cs = "A"; break;
	case IR_FLOAT_GREATER_EQUALS: cs = "AE"; break;
	case IR_FLOAT_LESS: cs = "B"; break;
	case IR_FLOAT_LESS_EQUALS: cs = "BE"; break;
	}

	return cs;
}

string varToString(IR_Value v)
{
	stringstream ss;

	string varType = "";
	if (v.type == IR_STRUCT)
	{
		varType = "STRUCT";
	}
	else if (v.type == IR_FLOAT)
	{
		varType = "FLOAT";
	}
	else {
		varType = "INT";
	}

	if (v.valueType == IR_LITERAL)
	{
		ss << "LITERAL (" << v.literalValue + ")";
	}
	else if (v.specialVars == IR_FLAGS)
	{
		ss << "%flags: " << conditionToString(v.flag);
	}
	else if (v.specialVars == IR_RETURN_INT || v.specialVars == IR_RETURN_FLOAT || v.specialVars == IR_RETURN_STACK)
	{
		ss << "%return - " << v.specialVars << " (" << varType << string(v.pointerLevel, '*') << ", " << v.byteSize << ")";
	}
	else {
		ss << "%" << (v.isTempValue ? "tmp" : "v") << v.varIndex << "(" << varType
			<< (v.pointerLevel > 0 ? string(v.pointerLevel, '*') : "") << ", " << v.byteSize << ")";
	}

	return ss.str();
}


string operandToString(IR_Operand operand)
{
	//TODO: add more!!!!
	stringstream ss;

	//set var type and byte size and pointer level based on if it's a derefenced value
	operand.value.byteSize = operand.value.byteSize;
	operand.value.type = operand.value.type;
	operand.value.pointerLevel = operand.value.pointerLevel;
	if (operand.dereference && operand.useMemoryAddress)
	{
		operand.dereference = false;
		operand.useMemoryAddress = false;
	}

	if (operand.globalFloatValue != 0)
	{
		ss << "[var #" << operand.globalFloatValue << "]";
		return ss.str();
	}

	if (operand.dereference)
	{
		ss << "[" << varToString(operand.value);
		if (operand.baseOffset != 0)
		{
			ss << " + " << operand.baseOffset;
		}
		if (operand.memoryOffsetMultiplier != 0)
		{
			if (operand.memoryOffsetMultiplier == 1)
			{
				ss << " + " << varToString(operand.memoryOffset);
			}
			else {
				ss << " + " << operand.memoryOffsetMultiplier << " * " << varToString(operand.memoryOffset);
			}
		}

		ss << "]";
	}
	else if (operand.useMemoryAddress)
	{
		ss << "Address(" << varToString(operand.value) << ")";
	}
	else {
		ss << varToString(operand.value);
	}
	return ss.str();
}


string IR_Assign::ToString()
{
	string assignType;
	switch (this->assignType)
	{
	case IR_ADD:
		assignType = "ADD";
		break;
	case IR_COPY:
		assignType = "COPY";
		break;
	case IR_DIVIDE:
		assignType = "DIV";
		break;
	case IR_FUSED_MULTIPLY_ADD:
		assignType = "FMA";
		break;
	case IR_LEFT_SHIFT:
		assignType = "L_SHIFT";
		break;
	case IR_MULTIPLY:
		assignType = "MUL";
		break;
	case IR_NEGATIVE:
		assignType = "NEG";
		break;
	case IR_RIGHT_SHIFT:
		assignType = "R_SHIFT";
		break;
	case IR_SUBTRACT:
		assignType = "SUB";
		break;
	case IR_TYPE_CAST:
		assignType = "TYPE_CAST";
		break;
	case IR_FLAG_CONVERT:
		assignType = "FLAG";
		break;
	case IR_STRUCT_COPY:
		assignType = "STRUCT_COPY";
		break;
	case IR_LEA:
		assignType = "LEA";
		break;
	}
	
	stringstream ss;
	ss << "Assign (" << byteSize << ") " << assignType << " " << operandToString(dest) << ", " << operandToString(source);
	return ss.str();
}

IR_Assign::IR_Assign() {}
IR_Assign::IR_Assign(IR_VarType type, IR_AssignType assignType, int byteSize, IR_Operand dest, IR_Operand source): type(type), assignType(assignType), byteSize(byteSize), dest(dest), source(source) {}
IR_StatementType IR_Assign::GetType()
{
	return _IR_ASSIGN;
}

void IR_Assign::ConvertToX64(x64_State& state)
{

	//deal with special cases first: int division, setting variable based on flag, 
	if (this->assignType == IR_DIVIDE && this->dest.GetVarType() == IR_INT)
	{
		//TODO: figure out int division
	}
	else if (this->assignType == IR_STRUCT_COPY)
	{
		this->dest.useMemoryAddress = true;
		this->source.useMemoryAddress = true;

		OperandAsm destOperand = IR_Statement::ConvertIrOperandToOperandAsm(this->dest, state);
		OperandAsm sourceOperand = IR_Statement::ConvertIrOperandToOperandAsm(this->source, state);

		state.StructCopy(destOperand, sourceOperand, this->byteSize);
		return;
	}
	StatementAsm assignStatement;

	OperandAsm firstOperandInitial = IR_Statement::ConvertIrOperandToOperandAsm(this->dest, state);
	assignStatement.firstOperand = firstOperandInitial;
	//set the destination register as modified for assign statement
	if (!assignStatement.firstOperand.dereference)
	{
		state.registerAllocator.registerMapping.regMapping.at((int)(assignStatement.firstOperand.reg.reg)).isModified = true;
	}
	assignStatement.secondOperand = IR_Statement::ConvertIrOperandToOperandAsm(this->source, state);

	bool isFloat = this->dest.GetVarType() == IR_FLOAT;
	switch (this->assignType)
	{
	case IR_COPY:
	{
		assignStatement.type = isFloat ? x64_MOVS : x64_MOV;
		break;
	}
	case IR_ADD:
	{
		assignStatement.type = isFloat ? x64_ADDS : x64_ADD;
		break;
	}
	case IR_SUBTRACT:
	{
		assignStatement.type = isFloat ? x64_SUBS : x64_SUB;
		break;
	}
	case IR_MULTIPLY:
	{
		assignStatement.type = isFloat ? x64_MULS : x64_IMUL;
		break;
	}
	case IR_DIVIDE:
	{
		assignStatement.type = x64_DIVS; //int division is special case that's already been taken care of above
		break;
	}
	case IR_LEA:
	{
		assignStatement.type = x64_LEA;
		break;
	}
	case IR_TYPE_CAST:
	{
		//isFloat is whether dest is float
		assignStatement.type = isFloat ? x64_CVTSI2SD : x64_CVTTSD2SI;
		break;
	}
	case IR_NEGATIVE:
	{
		//figure out negative of floats...
		break;
	}
	case IR_FLAG_CONVERT:
	{
		assignStatement.type = x64_SET;
		assignStatement.flags = this->source.value.flag;
		break;
	}
	}

	bool isArithmeticOperation = (this->assignType == IR_ADD || this->assignType == IR_SUBTRACT ||
		this->assignType == IR_MULTIPLY || this->assignType == IR_DIVIDE);
	bool replaceSourceOperand = assignStatement.secondOperand.dereference && ((isFloat && isArithmeticOperation) ||
		(assignStatement.firstOperand.dereference)
		|| (this->assignType == IR_TYPE_CAST));

	bool replaceDestOperand = (assignStatement.firstOperand.dereference && ((isFloat && isArithmeticOperation) ||
		((assignStatement.type == x64_IMUL || this->assignType == IR_TYPE_CAST || this->assignType == IR_FLAG_CONVERT))));

	if (replaceSourceOperand)
	{
		REGISTER tempReg = state.AllocateTempRegister(assignStatement.secondOperand, isFloat);
		assignStatement.secondOperand = OperandAsm::CreateRegisterOperand(tempReg);
	}

	REGISTER replaceDestRegister;
	if (replaceDestOperand)
	{
		replaceDestRegister = state.AllocateTempRegister(assignStatement.firstOperand, isFloat, this->assignType != IR_FLAG_CONVERT);
		assignStatement.firstOperand = OperandAsm::CreateRegisterOperand(replaceDestRegister);
	}

	if (this->assignType == IR_FLAG_CONVERT)
	{
		StatementAsm setToZero(x64_MOV);
		setToZero.firstOperand = OperandAsm::CreateRegisterOperand(assignStatement.firstOperand.reg.reg);
		setToZero.secondOperand = OperandAsm::CreateIntLiteralOperand(0);

		state.statements.push_back(std::move(setToZero));

		assignStatement.firstOperand.reg.size = 1; //use 1 byte version of register for set statement
	}

	state.statements.push_back(std::move(assignStatement));

	if (replaceDestOperand)
	{
		StatementAsm writeBackMov(isFloat ? x64_MOVS : x64_MOV);
		writeBackMov.firstOperand = firstOperandInitial;
		writeBackMov.secondOperand = OperandAsm::CreateRegisterOperand(replaceDestRegister);

		state.statements.push_back(std::move(writeBackMov));

		//clear register
		state.registerAllocator.registerMapping.regMapping.at((int)replaceDestRegister).variableIndex = 0;
	}
	
}

IR_VariableInit::IR_VariableInit() {}
IR_VariableInit::IR_VariableInit(IR_Value value) : dest(value) {}
string IR_VariableInit::ToString()
{
	return "INIT " + varToString(dest);
}

IR_StatementType IR_VariableInit::GetType()
{
	return _IR_VARIABLE_INIT;
}
void IR_VariableInit::ConvertToX64(x64_State& state)
{
	//TODO: Finish
}

string IR_Label::ToString()
{
	stringstream ss;
	ss << "Label #" << label << ":";
	return ss.str();
}
IR_StatementType IR_Label::GetType()
{
	return _IR_LABEL;
}
void IR_Label::ConvertToX64(x64_State& state)
{
	StatementAsm labelStatement(x64_LABEL);

	labelStatement.name = "label_" + std::to_string(this->label);

	//if previous instruction is jump, don't add the current register mapping to the label b/c it won't be accurate to control flow
	if (state.statements.back().type != x64_JMP)
	{
		if (isLoopLabel)
		{
			state.registerAllocator.SetInitialLabelMapping(this->label, state.registerAllocator.registerMapping);
		}
		else {
			//set this label x64 statement as the "jump label", so that storing statements will be done right before it
			state.registerAllocator.AddJumpLabelMapping(this->label, state.registerAllocator.registerMapping, state.statements.size());
		}
	}

	state.statements.push_back(std::move(labelStatement));

	if (isLoopLabel)
	{
		state.registerAllocator.startLoopLabelIndexes.push_back(LoopLabel(this->label, state.statements.size() - 1));
	}
	else {
		//find intersection of register mappings for non-loop labels (no back jumps to these labels so all control flow is known at this point)
		state.SetRegisterMappingToIntersectionOfMappings(label);
	}


}
IR_Label::IR_Label(int label, bool isLoopLabel) : label(label), isLoopLabel(isLoopLabel) {}


string IR_ScopeStart::ToString()
{
	return "Scope Start";
}
IR_StatementType IR_ScopeStart::GetType()
{
	return _IR_SCOPE_START;
}
void IR_ScopeStart::ConvertToX64(x64_State& state)
{
	state.irVariableData.irScopeStack.push_back(vector<int>());
}

string IR_ScopeEnd::ToString()
{
	return "Scope End";
}
IR_StatementType IR_ScopeEnd::GetType()
{
	return _IR_SCOPE_END;
}
void IR_ScopeEnd::ConvertToX64(x64_State& state)
{
	for (const int x : state.irVariableData.irScopeStack.back())
	{
		//find way to get size of all variables
		//don't just subtract because not variables will be spilled
		//only subtract for spilled variables
	/*	if (state.irVariableData.nonRegisterVariables.find(x) != state.irVariableData.nonRegisterVariables.end())
		{
			state.registerAllocator.currentStackPointerOffset -= state.irVariableData.nonRegisterVariables.at(x);
		}*/

		//TODO: subtract spilled variables
	}
	state.irVariableData.irScopeStack.pop_back();
}

string IR_Jump::ToString()
{
	stringstream ss;


	ss << "J" << conditionToString(condition) << " #" << labelIdx;
	return ss.str();
}
IR_StatementType IR_Jump::GetType()
{
	return _IR_JUMP;
}
void IR_Jump::ConvertToX64(x64_State& state)
{
	//add current register mapping to the vector of mappings at the label it's jumping to
	state.registerAllocator.AddJumpLabelMapping(labelIdx, state.registerAllocator.registerMapping, state.statements.size());

	StatementAsm jumpStatement(x64_JMP);
	jumpStatement.flags = this->condition;
	jumpStatement.name = "label_" + std::to_string(this->labelIdx);

	state.statements.push_back(jumpStatement);
}

IR_Jump::IR_Jump(int labelIdx, FlagResults condition): labelIdx(labelIdx), condition(condition) {}


IR_FunctionCall::IR_FunctionCall(string funcName, vector<IR_FunctionArgAssign>&& argAssignments) : funcName(funcName), 
	argAssignments(std::move(argAssignments)) {}
string IR_FunctionCall::ToString()
{
	stringstream ss;
	for (IR_FunctionArgAssign& funcAssign : this->argAssignments)
	{
		ss << funcAssign.ToString() << "\n";
	}
	ss << "CALL Function: " << this->funcName;
	return ss.str();
}
IR_StatementType IR_FunctionCall::GetType()
{
	return _IR_FUNCTION_CALL;
}
void IR_FunctionCall::ConvertToX64(x64_State& state)
{
	//assign variables to specfic registers
	//make sure to save call-clobbered registers as well (if they will be used afterwards)
	//save stack space for excess variables/ struct variables

	//after function call, add back value to stack pointer
	//clear out caller saved register mapping
	for (int i = 0; i < NUM_REGISTERS; ++i)
	{
		if (std::find(state._calleeSavedRegisters.begin(), state._calleeSavedRegisters.end(), (REGISTER)i) == state._calleeSavedRegisters.end())
		{
			if (state.registerAllocator.registerMapping.regMapping.at(i).variableIndex != 0)
			{
				state.SpillRegisterIfChanged(state.registerAllocator.registerMapping, (REGISTER)i);
				state.registerAllocator.registerMapping.regMapping.at(i).variableIndex = 0;
			}
		}
	}



	int totalOffset = 0;
	StatementAsm stackSubtract(x64_SUB, OperandAsm::CreateRegisterOperand(RSP), OperandAsm::CreateIntLiteralOperand(0));
	state.statements.push_back(std::move(stackSubtract));
	int stackSubtractStatementIdx = state.statements.size() - 1;


	for (IR_FunctionArgAssign& argAssign : this->argAssignments)
	{
		totalOffset = std::max(totalOffset, argAssign.stackArgOffset);
		if (argAssign.argType == IR_INT_ARG || argAssign.argType == IR_FLOAT_ARG)
		{
			bool isFloat = argAssign.argType == IR_FLOAT_ARG;
			const vector<REGISTER>& funcRegisters = isFloat ? state._functionFloatArguments : state._functionIntArguments;

			if (argAssign.argIdx < funcRegisters.size())
			{
				REGISTER reg = funcRegisters.at(argAssign.argIdx);
				//state.SpillRegisterIfChanged(state.registerAllocator.registerMapping, (int)reg);
				StatementAsm movStatement(isFloat ? x64_MOVS : x64_MOV);
				movStatement.firstOperand = OperandAsm::CreateRegisterOperand(reg);
				movStatement.secondOperand = IR_Statement::ConvertIrOperandToOperandAsm(argAssign.value, state);

				state.statements.push_back(std::move(movStatement));
			}
			else {
				StatementAsm movStatement(isFloat ? x64_MOVS : x64_MOV);
				movStatement.firstOperand = OperandAsm::CreateRSPOffsetOperand(argAssign.stackArgOffset);
				
				OperandAsm secondOperand = IR_Statement::ConvertIrOperandToOperandAsm(argAssign.value, state);
				if (secondOperand.dereference)
				{
					REGISTER reg = state.AllocateTempRegister(secondOperand, isFloat, true);
					secondOperand = OperandAsm::CreateRegisterOperand(reg);
				}
				movStatement.secondOperand = secondOperand;

				state.statements.push_back(std::move(movStatement));

			}
		}
		else {
			//state.StructCopy()
		}
	}



	StatementAsm functionCall(x64_CALL);
	functionCall.name = this->funcName;

	state.statements.push_back(std::move(functionCall));

	if (totalOffset == 0)
	{
		state.statements.at(stackSubtractStatementIdx).type = x64_NOP;
	}
	else {
		state.statements.at(stackSubtractStatementIdx).secondOperand.literalIntValue = totalOffset;
		StatementAsm stackAdd(x64_ADD, OperandAsm::CreateRegisterOperand(RSP),
			OperandAsm::CreateIntLiteralOperand(totalOffset));
		state.statements.push_back(std::move(stackAdd));
	}

}

//string IR_RegisterWriteToMemory::ToString()
//{
//	return "WRITE REGISTERS TO MEMORY";
//}
//IR_StatementType IR_RegisterWriteToMemory::GetType()
//{
//	return _IR_REGISTER_WRITE_TO_MEMORY;
//}

string IR_ContinuousMemoryInit::ToString()
{
	stringstream ss;
	ss << "Init Continuous Memory: %" << varIdx << " - " << byteNum << " bytes";
	return ss.str();
}
IR_StatementType IR_ContinuousMemoryInit::GetType()
{
	return _IR_CONTINUOUS_MEMORY_INIT;
}
void IR_ContinuousMemoryInit::ConvertToX64(x64_State& state)
{
	//TODO: Finish
}

string IR_FunctionArgAssign::ToString()
{
	stringstream ss;
	string type;
	if (this->argType == IR_INT_ARG)
	{
		type = "INT";
	}
	else if (this->argType == IR_FLOAT_ARG)
	{
		type = "FLOAT";
	}
	else {
		type = "STACK";
	}
	ss << "Arg " << type << " #" << this->argIdx << "(size: " << this->byteSize << ", offset: " << this->stackArgOffset << ")" << ": " << operandToString(this->value);
	return ss.str();
}
IR_StatementType IR_FunctionArgAssign::GetType()
{
	return _IR_FUNCTION_ARG_ASSIGN;
}
void IR_FunctionArgAssign::ConvertToX64(x64_State& state)
{
	//if (argType == IR_INT_ARG || argType == IR_FLOAT_ARG)
	//{
	//	bool isFloat = argType == IR_FLOAT_ARG;
	//	const vector<REGISTER>& funcRegisters = isFloat ? state._functionFloatArguments : state._functionIntArguments;

	//	if (this->argIdx < funcRegisters.size())
	//	{
	//		REGISTER reg = funcRegisters.at(argIdx);
	//		state.SpillRegisterIfChanged(state.registerAllocator.registerMapping, (int)reg);
	//		StatementAsm movStatement(isFloat ? x64_MOVS: x64_MOV);
	//		movStatement.firstOperand = OperandAsm::CreateRegisterOperand(reg);
	//		movStatement.secondOperand = IR_Statement::ConvertIrOperandToOperandAsm(this->value, state);

	//		state.statements.push_back(std::move(movStatement));
	//	}
	//	else {
	//		StatementAsm movStatement(isFloat ? x64_MOVS : x64_MOV);
	//		movStatement.firstOperand = OperandAsm::C
	//		movStatement.secondOperand = IR_Statement::ConvertIrOperandToOperandAsm(this->value, state);

	//		state.statements.push_back(std::move(movStatement));

	//	}
	//}
	//else {

	//}
}

IR_FunctionArgAssign::IR_FunctionArgAssign() {}
IR_FunctionArgAssign::IR_FunctionArgAssign(int argIdx, IR_Operand value, IR_FunctionArgType argType, int byteSize, int stackArgOffset): 
	argIdx(argIdx), value(value), argType(argType), byteSize(byteSize), stackArgOffset(stackArgOffset) {}


//string IR_VariableReload::ToString()
//{
//	return "VARIABLE RELOAD";
//}
//IR_StatementType IR_VariableReload::GetType()
//{
//	return _IR_VARIABLE_RELOAD;
//}

IR_Compare::IR_Compare() {}
IR_Compare::IR_Compare(IR_Operand op1, IR_Operand op2): op1(op1), op2(op2) {}

string IR_Compare::ToString()
{
	stringstream ss;

	ss << "Compare: " << operandToString(op1) << ", " << operandToString(op2);
	return ss.str();
}
IR_StatementType IR_Compare::GetType()
{
	return _IR_COMPARE;
}
void IR_Compare::ConvertToX64(x64_State& state)
{
	bool isFloat = op1.GetVarType() == IR_FLOAT;

	StatementAsm compareStatement(isFloat ? x64_COMIS : x64_CMP);
	OperandAsm firstOperand = IR_Statement::ConvertIrOperandToOperandAsm(op1, state);
	OperandAsm secondOperand = IR_Statement::ConvertIrOperandToOperandAsm(op2, state);

	REGISTER firstReg = _NONE;
	REGISTER secondReg = _NONE;
	if (isFloat)
	{
		if (firstOperand.dereference)
		{
			firstReg = state.AllocateTempRegister(firstOperand, isFloat);
			firstOperand = OperandAsm::CreateRegisterOperand(firstReg);
		}

		if (secondOperand.dereference)
		{
			secondReg = state.AllocateTempRegister(secondOperand, isFloat);
			secondOperand = OperandAsm::CreateRegisterOperand(secondReg);
		}
	}
	else {
		if (firstOperand.dereference && secondOperand.dereference)
		{
			secondReg = state.AllocateTempRegister(secondOperand, isFloat);
			secondOperand = OperandAsm::CreateRegisterOperand(secondReg);
		}
	}

	compareStatement.firstOperand = std::move(firstOperand);
	compareStatement.secondOperand = std::move(secondOperand);

	state.statements.push_back(std::move(compareStatement));
}

string IR_NOP::ToString()
{
	return "NOP";
}

IR_StatementType IR_NOP::GetType()
{
	return _IR_NOP;
}
void IR_NOP::ConvertToX64(x64_State& state)
{
	//TODO: Finish
}

string IR_LoopStart::ToString()
{
	return "LOOP START";
}
IR_StatementType IR_LoopStart::GetType()
{
	return _IR_LOOP_START;
}
void IR_LoopStart::ConvertToX64(x64_State& state)
{
	//TODO: Finish
}

string IR_LoopEnd::ToString()
{
	return "LOOP END";
}
IR_StatementType IR_LoopEnd::GetType()
{
	return _IR_LOOP_END;
}
void IR_LoopEnd::ConvertToX64(x64_State& state)
{
	LoopLabel startLoopLabel = state.registerAllocator.startLoopLabelIndexes.back();
	state.registerAllocator.startLoopLabelIndexes.pop_back();
	state.MatchRegisterMappingsToIntialMapping(startLoopLabel.labelIdx, startLoopLabel.labelStatementIdx);


}

string IR_FunctionLabel::ToString()
{
	stringstream ss;

	ss << "Function: " << this->functionName << "(";

	for (const IR_Value& value : this->args)
	{
		ss << varToString(value) << ", "; //note: this will add extra unnecessary comma (not important b/c this is just for debugging)
	}

	ss << ") -> ";
	ss << varToString(this->returnValue) << "(" << this->returnValueByteSize << ")";
	return ss.str();
}

IR_StatementType IR_FunctionLabel::GetType()
{
	return _IR_FUNCTION_LABEL;
}
void IR_FunctionLabel::ConvertToX64(x64_State& state)
{
	//TODO: Finish
}

IR_FunctionLabel::IR_FunctionLabel(string functionName) : functionName(functionName) {}
IR_FunctionLabel::IR_FunctionLabel() {}

string IR_Return::ToString()
{
	return "RET";
}

IR_StatementType IR_Return::GetType()
{
	return _IR_RETURN;
}
void IR_Return::ConvertToX64(x64_State& state)
{

	int functionStackSpace = 16 * ((state.registerAllocator.currentStackPointerOffset + 15) / 16); //round up to multiple of 16

	if (functionStackSpace == 0)
	{
		state.statements.at(state.registerAllocator.startFunctionStackPointerSubtractIndex).type = x64_NOP;
	}
	else {
		StatementAsm addRSP(x64_ADD);
		addRSP.firstOperand = OperandAsm::CreateRegisterOperand(RSP);
		addRSP.secondOperand = OperandAsm::CreateIntLiteralOperand(functionStackSpace); //this will be modified later

		state.statements.push_back(std::move(addRSP));

		//set the RSP subtract value to equal the value added at the end
		state.statements.at(state.registerAllocator.startFunctionStackPointerSubtractIndex).secondOperand.literalIntValue = functionStackSpace;
	}
	state.registerAllocator.startFunctionStackPointerSubtractIndex = -1;


	StatementAsm popRBP(x64_POP);
	popRBP.firstOperand = OperandAsm::CreateRegisterOperand(RBP);

	state.statements.push_back(std::move(popRBP));
	state.statements.push_back(StatementAsm(x64_RET));

}

string IR_FunctionStart::ToString()
{
	return "FUNCTION START";
}
IR_StatementType IR_FunctionStart::GetType()
{
	return _IR_FUNCTION_START;
}
void IR_FunctionStart::ConvertToX64(x64_State& state)
{
	//TODO: Add subtraction of RSP based on variables

	OperandAsm operandRBP = OperandAsm::CreateRegisterOperand(RBP);
	OperandAsm operandRSP = OperandAsm::CreateRegisterOperand(RSP);
	StatementAsm pushRBP(x64_PUSH);
	pushRBP.firstOperand = operandRBP;
	state.statements.push_back(std::move(pushRBP));

	StatementAsm movRSPtoRBP(x64_MOV);
	movRSPtoRBP.firstOperand = operandRBP;
	movRSPtoRBP.secondOperand = operandRSP;

	state.statements.push_back(std::move(movRSPtoRBP));

	StatementAsm subtractRSP(x64_SUB);
	subtractRSP.firstOperand = operandRSP;
	subtractRSP.secondOperand = OperandAsm::CreateIntLiteralOperand(0); //this will be modified later

	state.statements.push_back(std::move(subtractRSP));

	state.registerAllocator.startFunctionStackPointerSubtractIndex = state.statements.size() - 1;
}

string IR_FunctionEnd::ToString()
{
	return "FUNCTION END";
}
IR_StatementType IR_FunctionEnd::GetType()
{
	return _IR_FUNCTION_END;
}
void IR_FunctionEnd::ConvertToX64(x64_State& state)
{
	//TODO: Finish
}