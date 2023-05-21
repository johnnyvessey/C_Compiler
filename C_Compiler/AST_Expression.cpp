#include "AST_Expression.h"

using namespace AST_Expression;

Expression::Expression() : expressionType(ExpressionType::NONE), pointerLevel(0) {}



BinOp::BinOp(BinOpType type, int precedence) : type(type), precedence(precedence) {}

AST_BinOp::AST_BinOp() {
	expressionType = ExpressionType::BINARY_OPERATION;
}

void AST_BinOp::PrintExpressionAST(int indentLevel) {
	std::cout << string(indentLevel, '\t') << "Binary Operation: " << op << "\n";
	left->PrintExpressionAST(indentLevel + 1);
	right->PrintExpressionAST(indentLevel + 1);
}



void AST_Type_Cast_Expression::PrintExpressionAST(int indentLevel) {

}



AST_Function_Expression::AST_Function_Expression() {
	expressionType = ExpressionType::FUNCTION_CALL;
}

void AST_Function_Expression::PrintExpressionAST(int indentLevel) {

}



AST_Variable_Expression::AST_Variable_Expression(Variable v) : v(v)
{
	type = v.type;
}

void AST_Variable_Expression::PrintExpressionAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Variable: " << v.name << " (" << v.type << ") " << v.structName << "\n";
}


AST_Literal_Expression::AST_Literal_Expression() {
	expressionType = ExpressionType::LITERAL;
	pointerLevel = 0;
}

void AST_Literal_Expression::PrintExpressionAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Literal: " << value << "\n";
}




void AST_Struct_Variable_Access::PrintExpressionAST(int indentLevel)
{

}



void AST_Pointer_Dereference::PrintExpressionAST(int indentLevel)
{

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