#include "AST_Expression.h"

using namespace AST_Expression;

Expression::Expression() {}
IR_Value ParseBooleanExpression(Expression* expr, IR& irState, bool returnVar, shared_ptr<IR_Jump> prevTrueJump, shared_ptr<IR_Jump> prevFalseJump, bool invertResult);

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


IR_Value ConvertToBoolean(Expression* expr, IR& irState, bool invertResult)
{
	IR_Value value = expr->ConvertExpressionToIR(irState);
	const IR_Value zero(value.type, IR_LITERAL, value.byteSize, 0, true, "0");
	irState.IR_statements.push_back(make_shared<IR_Compare>(IR_Compare(IR_Operand(value), IR_Operand(zero))));
	IR_Value flagValue = irState.state.flags;
	flagValue.flag = invertResult ? IR_EQUALS : IR_NOT_EQUALS;
	return flagValue;
}

bool isAndOrExpression(Expression* expr)
{
	AST_BinOp* binOpExpr = dynamic_cast<AST_BinOp*>(expr);
	if (!binOpExpr) return false;
	return binOpExpr->op == BinOpType::AND || binOpExpr->op == BinOpType::OR;
}

IR_Value ParseAndOrReturnVar(AST_BinOp* binOpExpr, IR& irState, bool hasPrevAndOr, shared_ptr<IR_Jump>& prevTrueJump, shared_ptr<IR_Jump>& prevFalseJump, bool invertResult)
{
	int startIdx = irState.state.labelIndex++;
	int trueLabelIdx = irState.state.labelIndex++;
	int falseLabelIdx = irState.state.labelIndex++;
	int endLabelIdx = irState.state.labelIndex++;
	
	irState.IR_statements.push_back(make_shared<IR_Label>(startIdx));

	IR_Value result(IR_INT, IR_VARIABLE, 4, irState.state.tempVarIndex++, true, "", IR_NONE);

	IR_Value leftValue = ParseBooleanExpression(binOpExpr->left.get(), irState, false, nullptr, nullptr, false);

	const IR_Value zero(leftValue.type, IR_LITERAL, leftValue.byteSize, 0, true, "0");

	if (leftValue.specialVars != IR_FLAGS)
	{
		irState.IR_statements.push_back(make_shared<IR_Compare>(IR_Compare(IR_Operand(leftValue), IR_Operand(zero))));
	}

	IR_FlagResults newFlagResults;
	int shortCircuitLabelIdx;
	int nonShortCircuitLabelIdx;
	if (binOpExpr->op == BinOpType::AND)
	{
		newFlagResults = (IR_FlagResults)-leftValue.flag;
		shortCircuitLabelIdx = falseLabelIdx;
		if (hasPrevAndOr)
		{
			prevFalseJump->labelIdx = falseLabelIdx;
			prevTrueJump->labelIdx = startIdx;
		}

		irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(shortCircuitLabelIdx, newFlagResults)));
	}
	else { //OR
		newFlagResults = leftValue.flag;
		shortCircuitLabelIdx = trueLabelIdx;

		if (hasPrevAndOr)
		{
			prevTrueJump->labelIdx = trueLabelIdx;
			prevFalseJump->labelIdx = startIdx;
		}

		irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(shortCircuitLabelIdx, newFlagResults)));
	}

	IR_Value rightValue = ParseBooleanExpression(binOpExpr->right.get(), irState, false, nullptr, nullptr, false);

	if (rightValue.specialVars != IR_FLAGS)
	{
		irState.IR_statements.push_back(make_shared<IR_Compare>(IR_Compare(IR_Operand(rightValue), IR_Operand(zero))));
	}

	irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(trueLabelIdx, rightValue.flag)));
	//irState.IR_statements.push_back(make_unique<IR_Jump>(IR_Jump(falseLabelIdx, IR_ALWAYS))); //will be optimized out since FALSE label is right after this

	IR_Value falseIntLiteral(IR_INT, IR_LITERAL, 4, 0, true, "0", IR_NONE);
	IR_Value trueIntLiteral(IR_INT, IR_LITERAL, 4, 0, true, "1", IR_NONE);

	//FALSE Label:
	irState.IR_statements.push_back(make_shared<IR_Label>(IR_Label(falseLabelIdx)));
	irState.IR_statements.push_back(make_shared<IR_Assign>(IR_Assign(IR_INT, IR_COPY, IR_Operand(result), IR_Operand(invertResult ? trueIntLiteral : falseIntLiteral))));
	irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(endLabelIdx, IR_ALWAYS))); //maybe remove this, or let it be optimized out?


	//TRUE Label:
	irState.IR_statements.push_back(make_shared<IR_Label>(IR_Label(trueLabelIdx)));
	irState.IR_statements.push_back(make_shared<IR_Assign>(IR_Assign(IR_INT, IR_COPY, IR_Operand(result), IR_Operand(invertResult ? falseIntLiteral : trueIntLiteral))));
	//irState.IR_statements.push_back(make_unique<IR_Jump>(IR_Jump(endLabelIdx, IR_ALWAYS)));


	//END Label
	irState.IR_statements.push_back(make_shared<IR_Label>(IR_Label(endLabelIdx)));


	return result;
	
}

IR_Value ParseAndOrNoReturnVar(AST_BinOp* binOpExpr, IR& irState, bool hasPrevAndOr, shared_ptr<IR_Jump> prevTrueJump, shared_ptr<IR_Jump> prevFalseJump, bool invertResult)
{
	int startLabelIdx = irState.state.labelIndex++;
	int trueLabelIdx = irState.state.labelIndex++;
	int falseLabelIdx = irState.state.labelIndex++;


	irState.IR_statements.push_back(make_shared<IR_Label>(startLabelIdx));

	IR_Value leftValue = ParseBooleanExpression(binOpExpr->left.get(), irState, false, nullptr, nullptr, false);
	const IR_Value zero(leftValue.type, IR_LITERAL, leftValue.byteSize, 0, true, "0");

	if (leftValue.specialVars != IR_FLAGS)
	{
		irState.IR_statements.push_back(make_shared<IR_Compare>(IR_Compare(IR_Operand(leftValue), IR_Operand(zero))));
	}

	IR_FlagResults newFlagResults;
	int shortCircuitLabelIdx;
	int nonShortCircuitLabelIdx;
	shared_ptr<IR_Jump> trueJump = make_shared<IR_Jump>(IR_Jump(-1, IR_ALWAYS));
	shared_ptr<IR_Jump> falseJump = make_shared<IR_Jump>(IR_Jump(-1, IR_ALWAYS));

	if (binOpExpr->op == BinOpType::AND)
	{
		newFlagResults = (IR_FlagResults)-leftValue.flag;
		shortCircuitLabelIdx = falseLabelIdx;
		if (hasPrevAndOr)
		{
			prevFalseJump->labelIdx = falseLabelIdx;
			prevTrueJump->labelIdx = startLabelIdx;
		}

		irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(shortCircuitLabelIdx, newFlagResults)));
		//falseJump = dynamic_cast<IR_Jump*>(irState.IR_statements.back().get());

	}
	else { //OR
		newFlagResults = leftValue.flag;
		shortCircuitLabelIdx = trueLabelIdx;

		if (hasPrevAndOr)
		{
			prevTrueJump->labelIdx = trueLabelIdx;
			prevFalseJump->labelIdx = startLabelIdx;
		}

		irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(shortCircuitLabelIdx, newFlagResults)));
	}

	IR_Value rightValue = ParseBooleanExpression(binOpExpr->right.get(), irState, false, trueJump, falseJump, false);
	if (rightValue.specialVars != IR_FLAGS)
	{
		irState.IR_statements.push_back(make_shared<IR_Compare>(IR_Compare(IR_Operand(rightValue), IR_Operand(zero))));
	}


	//TRUE Label:
	irState.IR_statements.push_back(make_shared<IR_Label>(IR_Label(trueLabelIdx)));
	irState.IR_statements.push_back(invertResult ? falseJump : trueJump); //will be overwritten in next ParseAndOr call
	

	//FALSE Label:
	irState.IR_statements.push_back(make_shared<IR_Label>(IR_Label(falseLabelIdx)));

	irState.IR_statements.push_back(invertResult ? trueJump : falseJump); //will be overwritten in next ParseAndOr call	

	return irState.state.flags; //doesn't matter b/c this won't be used in the next part of the AND/OR chain

}

IR_Value ParseAndOr(AST_BinOp* binOpExpr, IR& irState, bool returnVar, shared_ptr<IR_Jump> prevTrueJump, shared_ptr<IR_Jump> prevFalseJump, bool invertResult)
{
	bool hasPrevAndOr = isAndOrExpression(binOpExpr->left.get());
	if (returnVar)
	{
		return ParseAndOrReturnVar(binOpExpr, irState, hasPrevAndOr, prevTrueJump, prevFalseJump, invertResult);
	}
	else {
		return ParseAndOrNoReturnVar(binOpExpr, irState, hasPrevAndOr, prevTrueJump, prevFalseJump, invertResult);
	}
}


IR_Value ConvertFlagsToTempVarConditionally(IR_Value flagValue, IR& irState, bool returnVar)
{
	if (returnVar)
	{
		IR_Value var(IR_INT, IR_VARIABLE, 4, irState.state.tempVarIndex++, true, "", IR_NONE);
		IR_Assign asssignFromFlags(IR_INT, IR_FLAG_CONVERT, var, flagValue);
		irState.IR_statements.push_back(make_shared<IR_Assign>(asssignFromFlags));
		return var;
	}
	else {
		return flagValue;
	}
}
IR_Value ParseBooleanExpression(Expression* expr, IR& irState, bool returnVar, shared_ptr<IR_Jump> prevTrueJump, shared_ptr<IR_Jump> prevFalseJump, bool invertResult)
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
			return ParseAndOr(binOpExpr, irState, returnVar, prevTrueJump, prevFalseJump, invertResult);
		}
		else if (IsComparisonOperation(binOpExpr->op))
		{
			IR_Value op1 = binOpExpr->left->ConvertExpressionToIR(irState);
			IR_Value op2 = binOpExpr->right->ConvertExpressionToIR(irState);

			irState.IR_statements.push_back(make_shared<IR_Compare>(IR_Compare(IR_Operand(op1), IR_Operand(op2))));

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
			IR_Value flagValue = ConvertToBoolean(expr, irState, invertResult);

			return ConvertFlagsToTempVarConditionally(flagValue, irState, returnVar);
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
		return ParseBooleanExpression(notExpr->expr.get(), irState, returnVar, prevTrueJump, prevFalseJump, !invertResult);

	}
	else {
		return ConvertToBoolean(expr, irState, invertResult);
	}

}

IR_Value AST_BinOp::ConvertExpressionToIR(IR& irState)
{

	//all expressions are temp values (for now)

	//if (leftValue.valueType == IR_LITERAL && rightValue.valueType == IR_LITERAL)
	//{
	//	//calculate literal value, store it in dest, and return regular assign
	//}
	 
	if (IR_Expression_Utils::irBinOpMapping.find(op) != IR_Expression_Utils::irBinOpMapping.end())
	{
		IR_Value leftValue = left->ConvertExpressionToIR(irState); //figure out whether there should be a second out parameter referring to return value of expression

		IR_Value dest(leftValue.type, IR_VARIABLE, leftValue.byteSize, irState.state.tempVarIndex++, true, "", IR_NONE);

		IR_AssignType binOpAssignType = IR_Expression_Utils::irBinOpMapping.at(op);

		//if it's result of boolean operation, convert it to int first
		if (leftValue.specialVars == IR_FLAGS)
		{
			IR_Assign assignFromFlags(IR_INT, IR_FLAG_CONVERT, dest, leftValue);
			irState.IR_statements.push_back(make_shared<IR_Assign>(std::move(assignFromFlags)));
		}
		else {
			IR_Assign copyLeft(dest.type, IR_AssignType::IR_COPY, dest, leftValue);
			irState.IR_statements.push_back(make_shared<IR_Assign>(std::move(copyLeft)));
		}

		IR_Value rightValue = right->ConvertExpressionToIR(irState);

		//if right value is boolean, convert it to int first
		if (rightValue.specialVars == IR_FLAGS)
		{
			IR_Value newRight(IR_INT, IR_VARIABLE, 4, irState.state.tempVarIndex++, true, "", IR_NONE);
			IR_Assign assignFromFlags(IR_INT, IR_FLAG_CONVERT, newRight, rightValue);
			irState.IR_statements.push_back(make_shared<IR_Assign>(std::move(assignFromFlags)));

			IR_Assign binOpStatement(dest.type, binOpAssignType, dest, newRight);
			irState.IR_statements.push_back(make_shared<IR_Assign>(std::move(binOpStatement)));
		}
		else
		{
			IR_Assign binOpStatement(dest.type, binOpAssignType, dest, rightValue);
			irState.IR_statements.push_back(make_shared<IR_Assign>(std::move(binOpStatement)));
		}

		return dest;

	}
	else if (IsBooleanOperation(op))
	{
		return ParseBooleanExpression(this, irState, true, nullptr, nullptr, false); //when using ConvertExpressionToIR, always make it return a variable (not just flags) for AND and OR
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

IR_Value AST_Type_Cast_Expression::ConvertExpressionToIR(IR& irState)
{
	
	IR_Value source = expr->ConvertExpressionToIR(irState);
	
	if (to.pointerLevel > 0 && from.pointerLevel > 0)
	{
		return source;
	}

	if (source.valueType == IR_LITERAL)
	{
		source.type = IR_Expression_Utils::irTypeMapping.at(to.lValueType); //check if type is different
		return source;
	}

	IR_Value dest;
	dest.varIndex = irState.state.tempVarIndex++;

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
	return dest;

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

IR_Value AST_Function_Expression::ConvertExpressionToIR(IR& irState)
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
		return irState.state.functionReturnValue;
	}
	else if (retType.lValueType != LValueType::STRUCT)
	{
		irState.state.functionReturnValue.byteSize = 4; //TODO: change this to be based on memory size of variable (not IR variable, because IR_INT can have multiple byte sizes)
		irState.state.functionReturnValue.type = retType.lValueType == INT ? IR_INT : IR_FLOAT; //assign return value to either RAX or XMM0 
		return irState.state.functionReturnValue;
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

IR_Value AST_Variable_Expression::ConvertExpressionToIR(IR& irState)
{
	return irState.state.scope.FindVariable(v.name);
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

IR_Value AST_Literal_Expression::ConvertExpressionToIR(IR& irState)
{
	//TODO: don't hard code in 4, figure out determining if INT vs LONG or FLOAT vs DOUBLE?
	return IR_Value(IR_Expression_Utils::irTypeMapping.at(this->type.lValueType), IR_LITERAL, 4, 0, true, this->value);
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

IR_Value AST_Struct_Variable_Access::ConvertExpressionToIR(IR& irState)
{
	//TODO: Finish this
	return IR_Value();

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

IR_Value AST_Pointer_Dereference::ConvertExpressionToIR(IR& irState)
{
	//TODO: Add IR statement WriteToMemory before adding IR statements for this 

		//TODO: Finish this
	return IR_Value();

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

IR_Value AST_Pointer_Offset::ConvertExpressionToIR(IR& irState)
{
	//TODO: Finish this
	return IR_Value();
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

IR_Value AST_Unary_Assignment_Expression::ConvertExpressionToIR(IR& irState)
{
	//TODO: if lvalue expression is pointer (pointerLevel > 0) or dereference of pointer, then add IR_VariableReload statement afterward
		//TODO: Finish this
	return IR_Value();

}

void AST_Negative_Expression::PrintExpressionAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Negative: "  << "\n";
	expr->PrintExpressionAST(indentLevel + 1);
}

AST_Negative_Expression::AST_Negative_Expression() {}
AST_Negative_Expression::AST_Negative_Expression(unique_ptr<Expression>&& expr) : expr(std::move(expr)) {}

IR_Value AST_Negative_Expression::ConvertExpressionToIR(IR& irState)
{
	IR_Value source = expr->ConvertExpressionToIR(irState);
	IR_Value dest = IR_Value(source.type, source.valueType, source.byteSize, irState.state.tempVarIndex++);
	irState.IR_statements.push_back(make_shared<IR_Assign>(IR_Assign(source.type, IR_NEGATIVE, dest, source)));

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

IR_Value AST_Address_Expression::ConvertExpressionToIR(IR& irState)
{
	//TODO: Finish this
	return IR_Value();


	
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

IR_Value AST_Not_Expression::ConvertExpressionToIR(IR& irState)
{
	//set invertResult to false, because it will be set to true inside the ParseBooleanExpression function
	return ParseBooleanExpression(this, irState, true, nullptr, nullptr, false); 
}

ExpressionType AST_Not_Expression::GetExpressionType()
{
	return ExpressionType::_Not_Expression;
}

ExpressionType AST_Assignment_Expression::GetExpressionType()
{
	return ExpressionType::_Assignment_Expression;
}

IR_Value AST_Assignment_Expression::ConvertExpressionToIR(IR& irState)
{
	
	//TODO: if lValue is pointer or dereference of pointer, add IR_VariableReload statement afterward

		//TODO: Finish this
	return IR_Value();

	
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