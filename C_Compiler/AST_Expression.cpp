#include "AST_Expression.h"

using namespace AST_Expression;

Expression::Expression() {}
IR_Operand ParseBooleanExpression(Expression* expr, IR& irState, bool returnVar, int& trueLabel, int& falseLabel, bool invertResult);

unordered_map<BinOpType, IR_AssignType> IR_Expression_Utils::irBinOpMapping = {
		{BinOpType::ADD, IR_AssignType::IR_ADD},
		{BinOpType::SUBTRACT, IR_AssignType::IR_SUBTRACT},
		{BinOpType::MULTIPLY, IR_AssignType::IR_MULTIPLY},
		{BinOpType::DIVIDE, IR_AssignType::IR_DIVIDE}
		//TODO: Add bit shifting and others
	};


 unordered_map<LValueType, IR_VarType> IR_Expression_Utils::irTypeMapping = {
	{FLOAT, IR_FLOAT},
	{INT, IR_INT}
};

//IR_Value ExpressionFunctions::GetValueFromExpression(unique_ptr<Expression>& expr, IR_State& irState)
//{
//	IR_Value result;
//	if (expr->GetExpressionType() == _Literal_Expression)
//	{
//		AST_Literal_Expression* literalExpr = ExpressionFunctions::GetSubexpressionPtr<AST_Literal_Expression>(expr);
//		result = (expr->type.lValueType == INT) ? IR_Utils::ParseLiteral(literalExpr->value, IR_INT) : IR_Utils::ParseLiteral(literalExpr->value, IR_FLOAT);
//	}
//	else if (expr->GetExpressionType() == _Function_Expression)
//	{
//		//IR_Value variable = irState.scope.FindVariable()
//
//	}
//
//}

 IR_Operand CopyDereferenceOfValue(IR_Operand baseValue, IR& irState)
 {
	 IR_Operand deref(IR_Value(baseValue.value.type, IR_VARIABLE, baseValue.value.byteSize, irState.state.varIndex++, true, "", IR_NONE));
	 deref.value.pointerLevel = baseValue.value.pointerLevel - 1;
	 if (deref.value.pointerLevel == 0)
	 {
		 deref.value.type = deref.value.baseType;
		 deref.value.byteSize = 4; //INT or FLOAT
	 }
	 irState.IR_statements.push_back(make_shared<IR_Assign>(IR_Assign(baseValue.value.type, IR_COPY, deref.value.byteSize, deref, baseValue)));

	 return deref;

 }


LValueExpression::LValueExpression()
{
	isLValue = true;
}


BinOp::BinOp(BinOpType type, int precedence) : type(type), precedence(precedence) {}

AST_BinOp::AST_BinOp() {
	isLValue = false;
}

void AST_BinOp::PrintExpressionAST(int indentLevel) {
	std::cout << string(indentLevel, '\t') << "Binary Operation: " << op << "\n";
	left->PrintExpressionAST(indentLevel + 1);
	right->PrintExpressionAST(indentLevel + 1);
}

ExpressionType AST_BinOp::GetExpressionType()
{
	return ExpressionType::_BinOp;
}


IR_Operand ConvertToBoolean(Expression* expr, IR& irState, bool invertResult)
{
	IR_Operand operand = expr->ConvertExpressionToIR(irState);
	const IR_Value zero(operand.value .type, IR_LITERAL, operand.value.byteSize, 0, true, "0");
	irState.IR_statements.push_back(make_shared<IR_Compare>(IR_Compare(operand, IR_Operand(zero))));
	IR_Value flagValue = irState.state.flags;
	flagValue.flag = invertResult ? IR_EQUALS : IR_NOT_EQUALS;
	return IR_Operand(flagValue);
}

bool isAndOrExpression(Expression* expr)
{
	AST_BinOp* binOpExpr = dynamic_cast<AST_BinOp*>(expr);
	if (!binOpExpr) return false;
	return binOpExpr->op == BinOpType::AND || binOpExpr->op == BinOpType::OR;
}

IR_Value ParseAndOrNoReturnVar(AST_BinOp* binOpExpr, IR& irState, int& trueLabelIdx, int& falseLabelIdx, bool invertResult)
{
	int insideTrueLabelIdx = trueLabelIdx;
	int insideFalseLabelIdx = falseLabelIdx;
	int outerTrueLabelIdx = irState.state.labelIndex++;
	int outerFalseLabelIdx = irState.state.labelIndex++;

	//set reference values, swap them if NOT operator is applied to it
	trueLabelIdx = !invertResult ? outerTrueLabelIdx : outerFalseLabelIdx;
	falseLabelIdx = !invertResult ? outerFalseLabelIdx : outerTrueLabelIdx;

	//this call sets the values of insideTrueLabelIdx and insideFalseLabelIdx
	IR_Operand leftValue = ParseBooleanExpression(binOpExpr->left.get(), irState, false, insideTrueLabelIdx, insideFalseLabelIdx, false);

	//TODO: Add inversion logic (logical NOT)
	if (isAndOrExpression(binOpExpr->left.get()))
	{
		irState.IR_statements.push_back(make_shared<IR_Label>(IR_Label(insideTrueLabelIdx)));
		if (binOpExpr->op == BinOpType::OR)
		{
			irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(outerTrueLabelIdx, IR_ALWAYS)));
		}
		irState.IR_statements.push_back(make_shared<IR_Label>(IR_Label(insideFalseLabelIdx)));
		if (binOpExpr->op == BinOpType::AND)
		{
			irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(outerFalseLabelIdx, IR_ALWAYS)));
		}

	}
	else {
		const IR_Value zero(leftValue.value.type, IR_LITERAL, leftValue.value.byteSize, 0, true, "0");

		if (leftValue.value.specialVars != IR_FLAGS)
		{
			irState.IR_statements.push_back(make_shared<IR_Compare>(IR_Compare(IR_Operand(leftValue), IR_Operand(zero))));
			leftValue.value.flag = IR_NOT_EQUALS; //consider if this is needed / has unintended side effects
		}

		if (binOpExpr->op == BinOpType::OR)
		{
			irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(outerTrueLabelIdx, leftValue.value.flag)));
			//irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(outerFalseLabelIdx, IR_ALWAYS)));

		}
		else {
			irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(outerFalseLabelIdx, (IR_FlagResults)-leftValue.value.flag)));
			//irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(outerTrueLabelIdx, IR_ALWAYS)));
		}

	}
	int insideTrueLabelIdxRight = trueLabelIdx;
	int insideFalseLabelIdxRight = falseLabelIdx;

	IR_Operand rightValue = ParseBooleanExpression(binOpExpr->right.get(), irState, false, insideTrueLabelIdxRight, insideFalseLabelIdxRight, false);
	if (isAndOrExpression(binOpExpr->right.get()))
	{
		//this will be optimized later so that there won't be multiple jumps in a row
		irState.IR_statements.push_back(make_shared<IR_Label>(IR_Label(insideTrueLabelIdxRight)));
		irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(outerTrueLabelIdx, IR_ALWAYS)));
		
		irState.IR_statements.push_back(make_shared<IR_Label>(IR_Label(insideFalseLabelIdxRight)));
		irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(outerFalseLabelIdx, IR_ALWAYS)));
	}
	else
	{
		if (rightValue.value.specialVars != IR_FLAGS)
		{
			const IR_Value zero(rightValue.value.type, IR_LITERAL, leftValue.value.byteSize, 0, true, "0");
			irState.IR_statements.push_back(make_shared<IR_Compare>(IR_Compare(IR_Operand(rightValue), IR_Operand(zero))));
			rightValue.value.flag = IR_NOT_EQUALS;
		}

		irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(trueLabelIdx, rightValue.value.flag)));
		irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(falseLabelIdx, IR_ALWAYS)));
	}

	//this value is irrelevant
	return irState.state.flags;
}

IR_Value ParseAndOrReturnVar(AST_BinOp* binOpExpr, IR& irState, int& trueLabel, int& falseLabel, bool invertResult)
{

	int endLabelIdx = irState.state.labelIndex++;
	IR_Value result(IR_INT, IR_VARIABLE, 4, irState.state.varIndex++, true, "", IR_NONE);

	ParseAndOrNoReturnVar(binOpExpr, irState, trueLabel, falseLabel, false); //don't invert the result because it will be done afterwards when setting value

	const IR_Value falseIntLiteral(IR_INT, IR_LITERAL, 4, 0, true, "0", IR_NONE);
	const IR_Value trueIntLiteral(IR_INT, IR_LITERAL, 4, 0, true, "1", IR_NONE);

	//FALSE Label:
	irState.IR_statements.push_back(make_shared<IR_Label>(IR_Label(falseLabel)));
	irState.IR_statements.push_back(make_shared<IR_Assign>(IR_Assign(IR_INT, IR_COPY, 4, IR_Operand(result), IR_Operand(invertResult ? trueIntLiteral : falseIntLiteral))));
	irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(endLabelIdx, IR_ALWAYS))); //maybe remove this, or let it be optimized out?


	//TRUE Label:
	irState.IR_statements.push_back(make_shared<IR_Label>(IR_Label(trueLabel)));
	irState.IR_statements.push_back(make_shared<IR_Assign>(IR_Assign(IR_INT, IR_COPY, 4, IR_Operand(result), IR_Operand(invertResult ? falseIntLiteral : trueIntLiteral))));
	irState.IR_statements.push_back(make_unique<IR_Jump>(IR_Jump(endLabelIdx, IR_ALWAYS))); //will be optimized out


	//END Label
	irState.IR_statements.push_back(make_shared<IR_Label>(IR_Label(endLabelIdx)));


	return result;

}


IR_Operand ParseAndOr(AST_BinOp* binOpExpr, IR& irState, bool returnVar, int& trueLabel, int& falseLabel, bool invertResult)
{
	bool hasPrevAndOr = isAndOrExpression(binOpExpr->left.get());
	if (returnVar)
	{
		return ParseAndOrReturnVar(binOpExpr, irState, trueLabel, falseLabel, invertResult);
	}
	else {
		return ParseAndOrNoReturnVar(binOpExpr, irState, trueLabel, falseLabel, invertResult);
	}
}


IR_Operand ConvertFlagsToTempVarConditionally(IR_Value flagValue, IR& irState, bool returnVar)
{
	if (returnVar)
	{
		IR_Value var(IR_INT, IR_VARIABLE, 4, irState.state.varIndex++, true, "", IR_NONE);
		IR_Assign asssignFromFlags(IR_INT, IR_FLAG_CONVERT, 4, var, flagValue);
		irState.IR_statements.push_back(make_shared<IR_Assign>(asssignFromFlags));
		return IR_Operand(var);
	}
	else {
		return IR_Operand(flagValue);
	}
}
IR_Operand ParseBooleanExpression(Expression* expr, IR& irState, bool returnVar, int& trueLabel, int& falseLabel, bool invertResult)
{
	if (expr->GetExpressionType() == _BinOp)
	{
		//parse AND + OR, as well as comparisons; other bin ops don't matter

		AST_BinOp* binOpExpr = dynamic_cast<AST_BinOp*>(expr);
		if (!binOpExpr)
		{
			//ERROR!
			throw 0;
		}
		if (binOpExpr->op == BinOpType::AND || binOpExpr->op == BinOpType::OR)
		{
			
			return ParseAndOr(binOpExpr, irState, returnVar, trueLabel, falseLabel, invertResult);
		}
		else if (IsComparisonOperation(binOpExpr->op))
		{
			IR_Operand op1 = binOpExpr->left->ConvertExpressionToIR(irState);
			IR_Operand op2 = binOpExpr->right->ConvertExpressionToIR(irState);

			irState.IR_statements.push_back(make_shared<IR_Compare>(IR_Compare(op1, op2)));

			IR_Value flagValue = irState.state.flags;
			//set flag results to use in later boolean expressions without having to convert it to another variable
			switch (binOpExpr->op)
			{
			case BinOpType::NOT_EQUALS: flagValue.flag = IR_FlagResults::IR_NOT_EQUALS; break;
			case BinOpType::EQUALS: flagValue.flag = IR_FlagResults::IR_EQUALS; break;
			case BinOpType::GREATER: flagValue.flag = IR_FlagResults::IR_GREATER; break;
			case BinOpType::GREATER_EQUAL: flagValue.flag = IR_FlagResults::IR_GREATER_EQUALS; break;
			case BinOpType::LESS: flagValue.flag = IR_FlagResults::IR_LESS; break;
			case BinOpType::LESS_EQUAL: flagValue.flag = IR_FlagResults::IR_LESS_EQUALS; break;
			}

			flagValue.flag = invertResult ? (IR_FlagResults) -flagValue.flag : flagValue.flag;

			return ConvertFlagsToTempVarConditionally(flagValue, irState, returnVar);


		}
		else {
			IR_Operand flagValue = ConvertToBoolean(expr, irState, invertResult);

			return ConvertFlagsToTempVarConditionally(flagValue.value, irState, returnVar);
		}
	}
	else if (expr->GetExpressionType() == _Not_Expression)
	{
		AST_Not_Expression* notExpr = dynamic_cast<AST_Not_Expression*>(expr);
		if (!notExpr)
		{
			//ERROR!!
			throw 0;
		}
		int trueLabel;
		int falseLabel;
		return ParseBooleanExpression(notExpr->expr.get(), irState, returnVar, trueLabel, falseLabel, !invertResult);

	}
	else {
		return ConvertToBoolean(expr, irState, invertResult);
	}

}

IR_Operand AST_BinOp::ConvertExpressionToIR(IR& irState)
{

	//all expressions are temp values (for now)

	//if (leftValue.valueType == IR_LITERAL && rightValue.valueType == IR_LITERAL)
	//{
	//	//calculate literal value, store it in dest, and return regular assign
	//}
	 
	if (IR_Expression_Utils::irBinOpMapping.find(op) != IR_Expression_Utils::irBinOpMapping.end())
	{
		IR_Operand leftValue = left->ConvertExpressionToIR(irState); //figure out whether there should be a second out parameter referring to return value of expression

		IR_Operand dest(IR_Value(leftValue.value.type, IR_VARIABLE, leftValue.value.byteSize, irState.state.varIndex++, true, "", IR_NONE));

		IR_AssignType binOpAssignType = IR_Expression_Utils::irBinOpMapping.at(op);

		//if it's result of boolean operation, convert it to int first
		if (leftValue.value.specialVars == IR_FLAGS)
		{
			IR_Assign assignFromFlags(IR_INT, IR_FLAG_CONVERT, 4, dest, leftValue);
			irState.IR_statements.push_back(make_shared<IR_Assign>(std::move(assignFromFlags)));
		}
		else {
			IR_Assign copyLeft(dest.value.type, IR_AssignType::IR_COPY, dest.GetByteSize(), dest, leftValue);
			irState.IR_statements.push_back(make_shared<IR_Assign>(std::move(copyLeft)));
		}

		IR_Operand rightValue = right->ConvertExpressionToIR(irState);

		//if right value is boolean, convert it to int first
		if (rightValue.value.specialVars == IR_FLAGS)
		{
			IR_Operand newRight(IR_Value(IR_INT, IR_VARIABLE, 4, irState.state.varIndex++, true, "", IR_NONE));
			IR_Assign assignFromFlags(IR_INT, IR_FLAG_CONVERT, 4, newRight, rightValue);
			irState.IR_statements.push_back(make_shared<IR_Assign>(std::move(assignFromFlags)));

			IR_Assign binOpStatement(dest.value.type, binOpAssignType, 4, dest, newRight);
			irState.IR_statements.push_back(make_shared<IR_Assign>(std::move(binOpStatement)));
		}
		else
		{
			IR_Assign binOpStatement(dest.value.type, binOpAssignType, 4, dest, rightValue);
			irState.IR_statements.push_back(make_shared<IR_Assign>(std::move(binOpStatement)));
		}

		return dest;

	}
	else if (IsBooleanOperation(op))
	{
		int trueLabel;
		int falseLabel;
		return ParseBooleanExpression(this, irState, true, trueLabel, falseLabel, false); //when using ConvertExpressionToIR, always make it return a variable (not just flags) for AND and OR
	}
	else {
		//pointer arith/ others
	}


	//return dest;
	
	
	//binOpStatement.assignType = IR_Utils::irBinOpMapping.at()

	//binOpStatement.result.isTempValue = true; //Look back at this... but it probably can be temp
	//if (right->GetExpressionType() == _Literal_Expression)
	//{
	//	AST_Literal_Expression* literalExpr = ExpressionFunctions::GetSubexpressionPtr<AST_Literal_Expression>(right);
	//	binOpStatement.result = (right->type.lValueType == INT) ? IR_Utils::ParseLiteral(literalExpr->value, IR_INT) : IR_Utils::ParseLiteral(literalExpr->value, IR_FLOAT);
	//}
	//else
	//{
	//	IR_Value variable = irState.state.scope.FindVariable()
	//}
	//binOpStatement.result.varIndex = irState.state.varIndex;
	//binOpStatement.result.type = 



}



bool AST_Type_Cast_Expression::IsValidTypeCast()
{
	return to.pointerLevel > 0 || (IsNumericType(from.lValueType) && IsNumericType(to.lValueType));
}


void AST_Type_Cast_Expression::PrintExpressionAST(int indentLevel) 
{
	std::cout << string(indentLevel, '\t') << "Type Cast:\n";
	std::cout << string(indentLevel + 1, '\t') << "From type: " << from.lValueType << " " << from.structName << " " << string(from.pointerLevel, '*') << "\n";
	std::cout << string(indentLevel + 1, '\t') << "To type: " << to.lValueType << " " << to.structName << " " << string(to.pointerLevel, '*') << "\n";
	expr->PrintExpressionAST(indentLevel + 1);
}

IR_Operand AST_Type_Cast_Expression::ConvertExpressionToIR(IR& irState)
{
	
	IR_Operand source = expr->ConvertExpressionToIR(irState);
	
	if (to.pointerLevel > 0 && from.pointerLevel > 0)
	{
		return source;
	}

	if (source.value.valueType == IR_LITERAL)
	{
		source.value.type = IR_Expression_Utils::irTypeMapping.at(to.lValueType); //check if type is different
		return source;
	}

	IR_Value dest;
	dest.varIndex = irState.state.varIndex++;

	IR_Assign typeCast;
	
	
	if (this->from.lValueType == INT && this->to.lValueType == FLOAT)
	{
		dest.byteSize = 4;
		dest.valueType = IR_VARIABLE;
		dest.type = IR_FLOAT;
	}
	else if (this->from.lValueType == FLOAT && this->to.lValueType == INT)
	{
		dest.byteSize = 4;
		dest.valueType = IR_VARIABLE;
		dest.type = IR_INT;
	}
	else {
		Utils::throwError("Haven't implemented this type cast yet");
	}

	typeCast.dest = dest;
	typeCast.source = std::move(source);
	typeCast.assignType = IR_TYPE_CAST;

	irState.IR_statements.push_back(make_unique<IR_Assign>(std::move(typeCast)));
	return IR_Operand(dest);

}

AST_Type_Cast_Expression::AST_Type_Cast_Expression() {}
AST_Type_Cast_Expression::AST_Type_Cast_Expression(unique_ptr<Expression>&& expr, VariableType from, VariableType to) : expr(std::move(expr)), from(from), to(to) {}


ExpressionType AST_Type_Cast_Expression::GetExpressionType()
{
	return ExpressionType::_Type_Cast_Expression;
}



AST_Function_Expression::AST_Function_Expression() {
	isLValue = false;
}

void AST_Function_Expression::PrintExpressionAST(int indentLevel) {
	std::cout << string(indentLevel, '\t') << "Function call: " << functionName << "\n";
	for (const unique_ptr<Expression>& param : argumentInstances)
	{
		param->PrintExpressionAST(indentLevel + 1);
	}
}

ExpressionType AST_Function_Expression::GetExpressionType()
{
	return ExpressionType::_Function_Expression;
}

IR_Operand AST_Function_Expression::ConvertExpressionToIR(IR& irState)
{
	//save registers to memory before calling function
	irState.IR_statements.push_back(make_unique<IR_RegisterWriteToMemory>());

	VariableType retType = irState.state.scope.functionMapping.at(this->functionName).returnType;

	int offset = 0;
	if (retType.lValueType == LValueType::STRUCT && retType.pointerLevel == 0)
	{
		//StructDefinition structDef = 
		//push back IR_Struct init statement (for return object
		//push back IR_FunctionArgAssign for pointer to struct location

		offset = 1;
	}
	for (int idx = 0; idx < argumentInstances.size(); ++idx)
	{
		irState.IR_statements.push_back(make_shared<IR_FunctionArgAssign>(IR_FunctionArgAssign(idx + offset, argumentInstances.at(idx)->ConvertExpressionToIR(irState))));
	}

	//reload variables after function call b/c values could have changed inside the function
	irState.IR_statements.push_back(make_shared<IR_VariableReload>());

	if (retType.pointerLevel > 0)
	{
		irState.state.functionReturnValue.byteSize = POINTER_SIZE;
		irState.state.functionReturnValue.type = IR_INT; //use int registers for pointer
		return IR_Operand(irState.state.functionReturnValue);
	}
	else if (retType.lValueType != LValueType::STRUCT)
	{
		irState.state.functionReturnValue.byteSize = 4; //TODO: change this to be based on memory size of variable (not IR variable, because IR_INT can have multiple byte sizes)
		irState.state.functionReturnValue.type = retType.lValueType == INT ? IR_INT : IR_FLOAT; //assign return value to either RAX or XMM0 
		return IR_Operand(irState.state.functionReturnValue);
	}
	else
	{

	}
}


AST_Variable_Expression::AST_Variable_Expression(Variable v) : v(v), LValueExpression()
{
	type = v.type;
}

void AST_Variable_Expression::PrintExpressionAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Variable: " << v.name << " (" << v.type.lValueType << ") " << v.type.structName << string(v.type.pointerLevel, '*') << "\n";
}

IR_Operand AST_Variable_Expression::ConvertExpressionToIR(IR& irState)
{
	return IR_Operand(irState.state.scope.FindVariable(v.name));
}

ExpressionType AST_Variable_Expression::GetExpressionType()
{
	return ExpressionType::_Variable_Expression;
}

AST_Literal_Expression::AST_Literal_Expression() {
	type.pointerLevel = 0;
	isLValue = false;
}

void AST_Literal_Expression::PrintExpressionAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Literal: " << value << "\n";
}

IR_Operand AST_Literal_Expression::ConvertExpressionToIR(IR& irState)
{
	//TODO: don't hard code in 4, figure out determining if INT vs LONG or FLOAT vs DOUBLE?
	return IR_Operand(IR_Value(IR_Expression_Utils::irTypeMapping.at(this->type.lValueType), IR_LITERAL, 4, 0, true, this->value));
}

ExpressionType AST_Literal_Expression::GetExpressionType()
{
	return ExpressionType::_Literal_Expression;
}

AST_Struct_Variable_Access::AST_Struct_Variable_Access(): LValueExpression()
{
}

void AST_Struct_Variable_Access::PrintExpressionAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Struct var access:\n";
	expr->PrintExpressionAST(indentLevel + 1);
	std::cout << string(indentLevel + 1, '\t') << "Access name: " << varName << "\n";
}

IR_Operand AST_Struct_Variable_Access::ConvertExpressionToIR(IR& irState)
{
	//TODO: Finish this
	return IR_Operand();

}

ExpressionType AST_Struct_Variable_Access::GetExpressionType()
{
	return ExpressionType::_Struct_Variable_Access;
}


void AST_Pointer_Dereference::PrintExpressionAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Dereference:" << "\n";
	baseExpr->PrintExpressionAST(indentLevel + 1);
}

IR_Operand AST_Pointer_Dereference::ConvertExpressionToIR(IR& irState)
{
	//TODO: Add IR statement WriteToMemory before adding IR statements for this 

	/*Expression* expr = this->baseExpr.get();
	while (expr->GetExpressionType() == _Pointer_Dereference)
	{
		expr = (dynamic_cast<AST_Pointer_Dereference*>(expr))->baseExpr.get();
	}*/

	IR_Operand baseExprOperand;
	if (baseExpr->GetExpressionType() == _Pointer_Offset)
	{
		AST_Pointer_Offset* pointerOffsetExpr = dynamic_cast<AST_Pointer_Offset*>(baseExpr.get());
		baseExprOperand = pointerOffsetExpr->expr->ConvertExpressionToIR(irState);
		IR_Operand offsetOperand = pointerOffsetExpr->index->ConvertExpressionToIR(irState);

		VariableType type = pointerOffsetExpr->type;
		--type.pointerLevel;

		StructDefinition* structDef = nullptr;
		int memoryMultiplier = GetMemorySizeForIR(type, structDef);

		if (offsetOperand.value.valueType == IR_LITERAL)
		{
			if (type.lValueType == LValueType::STRUCT)
			{
				*structDef = irState.state.scope.FindStruct(type.structName);
			}
			baseExprOperand.baseOffset += (std::stoi(offsetOperand.value.literalValue) *  memoryMultiplier);
		}
		else {

			IR_Operand offsetOperandFinal = offsetOperand.dereference ? CopyDereferenceOfValue(offsetOperand, irState) : offsetOperand;

			if (memoryMultiplier == 1 || memoryMultiplier == 2 || memoryMultiplier == 4 || memoryMultiplier == 8)
			{
				baseExprOperand.memoryOffsetMultiplier = memoryMultiplier;
				baseExprOperand.memoryOffset = offsetOperandFinal.value;
			}
			else {
				IR_Operand offsetFinal(IR_Value(IR_INT, IR_VARIABLE, 4, irState.state.varIndex++, true, "", IR_NONE));
				irState.IR_statements.push_back(make_shared<IR_Assign>(IR_Assign(IR_INT, IR_COPY, 4, offsetFinal, offsetOperandFinal)));
				irState.IR_statements.push_back(make_shared<IR_Assign>(IR_Assign(IR_INT, IR_MULTIPLY, 4, offsetFinal, 
					IR_Operand(IR_Value(IR_INT, IR_LITERAL, 4, 0, true, std::to_string(memoryMultiplier), IR_NONE)))));

				baseExprOperand.memoryOffsetMultiplier = 1;
				baseExprOperand.memoryOffset = offsetFinal.value;
			}
		}

	}
	IR_Operand baseValue = (baseExpr->GetExpressionType() == _Pointer_Offset) ? baseExprOperand : this->baseExpr->ConvertExpressionToIR(irState);
	if (baseValue.dereference == false)
	{
		baseValue.dereference = true;
		return baseValue;
	}
	else {
		IR_Operand derefValue = CopyDereferenceOfValue(baseValue, irState);
		derefValue.dereference = true;
		return derefValue;
	}
	
}

ExpressionType AST_Pointer_Dereference::GetExpressionType()
{
	return ExpressionType::_Pointer_Dereference;
}

AST_Pointer_Dereference::AST_Pointer_Dereference(): LValueExpression() {
}
AST_Pointer_Dereference::AST_Pointer_Dereference(unique_ptr<Expression>&& expr) {
	baseExpr = std::move(expr);
	type = VariableType(baseExpr->type.lValueType, baseExpr->type.structName, baseExpr->type.pointerLevel - 1);
}

//AST_Array_Index::AST_Array_Index(unique_ptr<Expression>&& expr): expr(std::move(expr))
//{
//	
//}



ExpressionType AST_Pointer_Offset::GetExpressionType()
{
	return ExpressionType::_Pointer_Offset;
}

void AST_Pointer_Offset::PrintExpressionAST(int indentLevel)
{
	std::cout << string(indentLevel,'\t') << "Pointer offset:\n";

	std::cout << string(indentLevel + 1, '\t') << "Base expression:\n";
	expr->PrintExpressionAST(indentLevel + 2);

	std::cout << string(indentLevel + 1, '\t') << "Index:\n";
	index->PrintExpressionAST(indentLevel + 2);

}

IR_Operand AST_Pointer_Offset::ConvertExpressionToIR(IR& irState)
{
	//TODO: Finish this
	return IR_Operand();
}

void AST_Unary_Assignment_Expression::PrintExpressionAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Unary Assigment (" << opType << ", " << (isPrefix ? "Prefix" : "Postfix") << "):\n";
	expr->PrintExpressionAST(indentLevel + 1);
}

ExpressionType AST_Unary_Assignment_Expression::GetExpressionType()
{
	return ExpressionType::_Unary_Assignment_Expression;
}

IR_Operand AST_Unary_Assignment_Expression::ConvertExpressionToIR(IR& irState)
{
	//TODO: if lvalue expression is pointer (pointerLevel > 0) or dereference of pointer, then add IR_VariableReload statement afterward
		//TODO: Finish this
	return IR_Operand();

}

void AST_Negative_Expression::PrintExpressionAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Negative: "  << "\n";
	expr->PrintExpressionAST(indentLevel + 1);
}

AST_Negative_Expression::AST_Negative_Expression() {}
AST_Negative_Expression::AST_Negative_Expression(unique_ptr<Expression>&& expr) : expr(std::move(expr)) {}

IR_Operand AST_Negative_Expression::ConvertExpressionToIR(IR& irState)
{
	IR_Operand source = expr->ConvertExpressionToIR(irState);
	IR_Operand dest = IR_Operand(IR_Value(source.value.type, source.value.valueType, source.value.byteSize, irState.state.varIndex++));
	irState.IR_statements.push_back(make_shared<IR_Assign>(IR_Assign(source.value.type, IR_NEGATIVE, source.GetByteSize(), dest, source)));

	return dest;
}

ExpressionType AST_Negative_Expression::GetExpressionType()
{
	return ExpressionType::_Negative_Expression;
}

void AST_Address_Expression::PrintExpressionAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Address of: " << "\n";
	expr->PrintExpressionAST(indentLevel + 1);
}

IR_Operand AST_Address_Expression::ConvertExpressionToIR(IR& irState)
{
	//TODO: Finish this
	return IR_Operand();


	
}

ExpressionType AST_Address_Expression::GetExpressionType()
{
	return ExpressionType::_Address_Expression;
}

void AST_Not_Expression::PrintExpressionAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Not: " << "\n";
	expr->PrintExpressionAST(indentLevel + 1);
}

IR_Operand AST_Not_Expression::ConvertExpressionToIR(IR& irState)
{
	int trueLabel;
	int falseLabel;
	//set invertResult to false, because it will be set to true inside the ParseBooleanExpression function
	return ParseBooleanExpression(this, irState, true, trueLabel, falseLabel, false); 
}

ExpressionType AST_Not_Expression::GetExpressionType()
{
	return ExpressionType::_Not_Expression;
}

ExpressionType AST_Assignment_Expression::GetExpressionType()
{
	return ExpressionType::_Assignment_Expression;
}

IR_AssignType GetAssignTypeFromToken(TokenType token)
{
	switch (token)
	{
	case PLUS_EQUAL: return IR_ADD;
	case MINUS_EQUAL: return IR_SUBTRACT;
	case STAR_EQUAL: return IR_MULTIPLY;
	case SLASH_EQUAL: return IR_DIVIDE;
	case SINGLE_EQUAL: return IR_COPY;
	}

	std::cout << "can't find assign type\n";
	throw 0;
}

IR_Operand AST_Assignment_Expression::ConvertExpressionToIR(IR& irState)
{
	
	//TODO: if lValue is pointer or dereference of pointer, add IR_VariableReload statement afterward

	IR_Operand lValue = this->lvalue->ConvertExpressionToIR(irState);
	IR_Operand rValue = this->rvalue->ConvertExpressionToIR(irState);

	IR_Assign assign;
	assign.dest = lValue;
	assign.type = lValue.GetVarType();

	if (rValue.value.specialVars == IR_FLAGS)
	{

		//be careful when copying flag (1 byte) to memory; set it to register first and then copy 4 bytes over
		//TODO: see if I need to change logic of IR_FLAG_CONVERT, maybe I don't need this check 
		//and the rest of the managing of registers will be done in x64 translation phase
		if (lValue.dereference)
		{
			IR_Operand tempValue(IR_Value(lValue.GetVarType(), IR_VARIABLE, 4, irState.state.varIndex++, true, "", IR_NONE));
			irState.IR_statements.push_back(make_shared<IR_Assign>(IR_Assign(tempValue.value.type, IR_FLAG_CONVERT, 4, tempValue, rValue)));
			assign.assignType = IR_COPY;
			assign.source = tempValue;
		}
		else {
			assign.assignType == IR_FLAG_CONVERT;
			assign.source = rValue;
		}
	}
	else {
		assign.assignType = GetAssignTypeFromToken(assignmentOperator);
		if (lValue.dereference && rValue.dereference)
		{
			IR_Operand derefRValue = CopyDereferenceOfValue(rValue, irState);
			assign.source = derefRValue;
		}
		else {
			assign.source = rValue;
		}
	}


	irState.IR_statements.push_back(make_shared<IR_Assign>(std::move(assign)));
	return lValue;

}

void AST_Assignment_Expression::PrintExpressionAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Assign (op = " << assignmentOperator << "):\n";
	lvalue->PrintExpressionAST(indentLevel + 1);
	rvalue->PrintExpressionAST(indentLevel + 1);
}



unordered_map<TokenType, BinOp> ExpressionUtils::BinOpTokenDictionary = {
	{TokenType::PLUS, BinOp(BinOpType::ADD, 300)},
	{TokenType::MINUS, BinOp(BinOpType::SUBTRACT, 300)},
	{TokenType::STAR, BinOp(BinOpType::MULTIPLY, 400)},
	{TokenType::SLASH, BinOp(BinOpType::DIVIDE, 400)},
	{TokenType::PERCENT, BinOp(BinOpType::MODULO, 400)},
	{TokenType::AND, BinOp(BinOpType::AND, 200)},
	{TokenType::OR, BinOp(BinOpType::OR, 150)},
	{TokenType::DOUBLE_EQUAL, BinOp(BinOpType::EQUALS, 180)},
	{TokenType::NOT_EQUALS, BinOp(BinOpType::NOT_EQUALS, 180)},
	{TokenType::LESS_THAN, BinOp(BinOpType::LESS, 190)},
	{TokenType::GREATER_THAN, BinOp(BinOpType::GREATER, 190)},
	{TokenType::LESS_THAN_EQUALS, BinOp(BinOpType::LESS_EQUAL, 190)},
	{TokenType::GREATER_THAN_EQUALS, BinOp(BinOpType::GREATER_EQUAL, 190)}

	//TODO: Continue this for comparison and boolean
};

unordered_map<TokenType, BinOpType> ExpressionUtils::BinOpAssignmentTypeDictionary = {
	{TokenType::PLUS_EQUAL, BinOpType::ADD},
	{TokenType::MINUS_EQUAL, BinOpType::SUBTRACT},
	{TokenType::STAR_EQUAL, BinOpType::MULTIPLY},
	{TokenType::SLASH_EQUAL, BinOpType::DIVIDE},
	{TokenType::PERCENT_EQUAL, BinOpType::MODULO}
};



unordered_map<TokenType, LValueType> ExpressionUtils::TokenTypeToLValueTypeMapping = {
	{TokenType::INT_LITERAL, LValueType::INT},
	{TokenType::FLOAT_LITERAL, LValueType::FLOAT}
};