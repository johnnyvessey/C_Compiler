#pragma once

#include <string>
#include <memory>
#include <iostream>
#include <unordered_map>

#include "Lexer.h"
#include "Variable.h"

using std::string;
using std::unique_ptr;
using std::unordered_map;

using namespace Lexer;
using namespace VariableNamespace;

namespace AST_Expression
{
	enum ExpressionType {
		NONE,
		LITERAL,
		BINARY_OPERATION,
		UNARY_OPERATION,
		FUNCTION_CALL,
		VARIABLE
	};


	enum BinOpType {
		ADD,
		SUBTRACT,
		MULTIPLY,
		DIVIDE,
		MODULO,
		GREATER,
		GREATER_EQUAL,
		LESS,
		LESS_EQUAL,
		EQUALS,
		NOT_EQUALS,
		AND,
		OR,
		POINTER_OFFSET, //e.g.: int *ptr; ptr = ptr + 1;
		BIT_SHIFT_LEFT, //TODO: ADD THESE
		BIT_SHIFT_RIGHT
	};

	bool IsBooleanOperation(BinOpType type)
	{
		return type == EQUALS || type == NOT_EQUALS || type == LESS || type == LESS_EQUAL || type == GREATER || type == GREATER_EQUAL || type == AND || type == OR;
	}

	struct BinOp {
		BinOpType type;
		int precedence;

		BinOp(BinOpType type, int precedence) : type(type), precedence(precedence) {}
	};

	unordered_map<TokenType, BinOp> BinOpTokenDictionary = {
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


	struct Expression {
		LValueType type;
		string structName;
		bool isReference;
		ExpressionType expressionType;

		Expression()
		{
			expressionType = ExpressionType::NONE;
			isReference = false;
		}

		virtual void PrintExpressionAST(int indentLevel = 0) = 0;
	};

	struct AST_BinOp : Expression {
		BinOpType op;
		unique_ptr<Expression> left;
		unique_ptr<Expression> right;

		AST_BinOp() {
			expressionType = ExpressionType::BINARY_OPERATION;
		}

		virtual void PrintExpressionAST(int indentLevel = 0) override;
	};

	void AST_BinOp::PrintExpressionAST(int indentLevel) {
		std::cout << string(indentLevel, '\t') << "Binary Operation: " << op << "\n";
		left->PrintExpressionAST(indentLevel + 1);
		right->PrintExpressionAST(indentLevel + 1);
	}

	struct AST_Type_Cast_Expression : Expression
	{
		//figure out pointer casting!! and how to represent pointers. Maybe have int count of number of pointers (i.e: value of 3 for int*** x;)
		LValueType from;
		LValueType to;
		virtual void PrintExpressionAST(int indentLevel = 0) override;
	};

	void AST_Type_Cast_Expression::PrintExpressionAST(int indentLevel) {

	}


	struct AST_Function_Expression : Expression
	{

		AST_Function_Expression() {
			expressionType = ExpressionType::FUNCTION_CALL;
		}

		virtual void PrintExpressionAST(int indentLevel = 0) override;
	};

	void AST_Function_Expression::PrintExpressionAST(int indentLevel) {

	}

	//static unordered_map<TokenType, BinOpType> BinOpDict = {
	//{TokenType::PLUS, BinOpType::ADD},
	//{TokenType::MINUS, BinOpType::SUBTRACT},
	//{TokenType::STAR, BinOpType::MULTIPLY},
	//{TokenType::SLASH, BinOpType::DIVIDE},
	//{TokenType::PERCENT, BinOpType::MODULO},
	//};

	struct BinOperator {
		BinOpType type;
	};




	static unordered_map<TokenType, LValueType> TokenTypeToLValueTypeMapping = {
		{TokenType::INT_LITERAL, LValueType::INT},
		{TokenType::FLOAT_LITERAL, LValueType::FLOAT}
	};

	struct AST_Variable_Expression : Expression 
	{
		Variable v;
		virtual void PrintExpressionAST(int indentLevel = 0) override;

		AST_Variable_Expression(Variable&& v) : v(v) {
			type = v.type;
		}
	};

	void AST_Variable_Expression::PrintExpressionAST(int indentLevel)
	{
		std::cout << string(indentLevel, '\t') << "Variable: " << v.name << " (" << v.type << ") " << v.structName << "\n";
	}


	struct AST_Literal_Expression : Expression {

		string value;

		AST_Literal_Expression() {
			expressionType = ExpressionType::LITERAL;
			isReference = false;
		}

		virtual void PrintExpressionAST(int indentLevel = 0) override;
	};

	void AST_Literal_Expression::PrintExpressionAST(int indentLevel)
	{
		std::cout << string(indentLevel, '\t') << "Literal: " << value << "\n";
	}

	struct AST_Struct_Variable_Access : Expression {

		virtual void PrintExpressionAST(int indentLevel = 0) override;
	};

	void AST_Struct_Variable_Access::PrintExpressionAST(int indentLevel)
	{

	}

	struct AST_Pointer_Dereference : Expression
	{
		virtual void PrintExpressionAST(int indentLevel = 0) override;
	};

	void AST_Pointer_Dereference::PrintExpressionAST(int indentLevel)
	{

	}

}