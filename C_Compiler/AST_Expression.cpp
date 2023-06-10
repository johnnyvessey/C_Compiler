#include "AST_Expression.h"

using namespace AST_Expression;

Expression::Expression() {}

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


AST_Variable_Expression::AST_Variable_Expression(Variable v) : v(v), LValueExpression()
{
	type = v.type;
}

void AST_Variable_Expression::PrintExpressionAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Variable: " << v.name << " (" << v.type.lValueType << ") " << v.type.structName << string(v.type.pointerLevel, '*') << "\n";
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

ExpressionType AST_Struct_Variable_Access::GetExpressionType()
{
	return ExpressionType::_Struct_Variable_Access;
}


void AST_Pointer_Dereference::PrintExpressionAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Dereference:" << "\n";
	baseExpr->PrintExpressionAST(indentLevel + 1);
}

ExpressionType AST_Pointer_Dereference::GetExpressionType()
{
	return ExpressionType::_Pointer_Dereference;
}

AST_Pointer_Dereference::AST_Pointer_Dereference(): LValueExpression() {
}
AST_Pointer_Dereference::AST_Pointer_Dereference(unique_ptr<Expression>&& expr) : baseExpr(std::move(expr)) {}

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

void AST_Unary_Assignment_Expression::PrintExpressionAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Unary Assigment (" << opType << ", " << (isPrefix ? "Prefix" : "Postfix") << "):\n";
	expr->PrintExpressionAST(indentLevel + 1);
}

ExpressionType AST_Unary_Assignment_Expression::GetExpressionType()
{
	return ExpressionType::_Unary_Assignment_Expression;
}

void AST_Negative_Expression::PrintExpressionAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Negative: "  << "\n";
	expr->PrintExpressionAST(indentLevel + 1);
}

AST_Negative_Expression::AST_Negative_Expression() {}
AST_Negative_Expression::AST_Negative_Expression(unique_ptr<Expression>&& expr) : expr(std::move(expr)) {}


ExpressionType AST_Negative_Expression::GetExpressionType()
{
	return ExpressionType::_Negative_Expression;
}

void AST_Address_Expression::PrintExpressionAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Address of: " << "\n";
	expr->PrintExpressionAST(indentLevel + 1);
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

ExpressionType AST_Not_Expression::GetExpressionType()
{
	return ExpressionType::_Not_Expression;
}

ExpressionType AST_Assignment_Expression::GetExpressionType()
{
	return ExpressionType::_Assignment_Expression;
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