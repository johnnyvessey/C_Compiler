#include "AST_Expression.h"

using namespace AST_Expression;

Expression::Expression() {}

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

namespace IR_Utils {

	const static unordered_map<BinOpType, IR_AssignType> irBinOpMapping = {
		{BinOpType::ADD, IR_AssignType::IR_ADD},
		{BinOpType::SUBTRACT, IR_AssignType::IR_SUBTRACT},
		{BinOpType::MULTIPLY, IR_AssignType::IR_MULTIPLY},
		{BinOpType::DIVIDE, IR_AssignType::IR_DIVIDE}
		//TODO: Add bit shifting and others

	};
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


IR_Value AST_BinOp::ConvertExpressionToIR(IR& irState)
{

	IR_Value leftValue = left->ConvertExpressionToIR(irState); //figure out whether there should be a second out parameter referring to return value of expression
	IR_Value rightValue = right->ConvertExpressionToIR(irState);
	 

	IR_Value dest;
	dest.varIndex = irState.state.varIndex++; //increment after setting it
	dest.byteSize = leftValue.byteSize;
	dest.type = leftValue.type;

	//all expressions are temp values (for now)
	dest.isTempValue = true;

	if (leftValue.valueType == IR_LITERAL && rightValue.valueType == IR_LITERAL)
	{
		//calculate literal value, store it in dest, and return regular assign
	}
	else {
		if (IR_Utils::irBinOpMapping.find(op) != IR_Utils::irBinOpMapping.end())
		{
			IR_AssignType binOpAssignType = IR_Utils::irBinOpMapping.at(op);
			IR_Assign copyLeft(dest.type, IR_AssignType::COPY, dest, leftValue);
			IR_Assign binOpStatement(dest.type, binOpAssignType, dest, rightValue);
			
			irState.IR_statements.push_back(make_unique<IR_Assign>(std::move(copyLeft)));
			irState.IR_statements.push_back(make_unique<IR_Assign>(std::move(binOpStatement)));

		}
		else {
			//pointer arithmetic, etc...
		}


		return dest;
	}
	
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