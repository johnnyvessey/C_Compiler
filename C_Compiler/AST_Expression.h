#pragma once

#include <string>
#include <memory>
#include <iostream>
#include <unordered_map>

#include "Lexer.h"

using std::string;
using std::unique_ptr;
using std::unordered_map;

using namespace Lexer;

namespace AST_Expression
{
	enum ExpressionType {
		NONE,
		LITERAL,
		BINARY_OPERATION,
		UNARY_OPERATION,
		FUNCTION_CALL
	};

	enum BinOpType {
		ADD,
		SUBTRACT,
		MULTIPLY,
		DIVIDE,
		MODULO
	};

	enum LValueType {
		INT,
		FLOAT,
		STRUCT
	};


	struct Expression {
		LValueType type;
		string structName;
		bool isPointer;
		ExpressionType expressionType;

		Expression()
		{
			expressionType = ExpressionType::NONE;
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

	static unordered_map<TokenType, BinOpType> BinOpDict = {
	{TokenType::PLUS, BinOpType::ADD},
	{TokenType::MINUS, BinOpType::SUBTRACT},
	{TokenType::STAR, BinOpType::MULTIPLY},
	{TokenType::SLASH, BinOpType::DIVIDE},
	{TokenType::PERCENT, BinOpType::MODULO}
	};

	struct BinOperator {
		BinOpType type;
	};




	static unordered_map<TokenType, LValueType> TokenTypeToLValueTypeMapping = {
		{TokenType::INT_LITERAL, LValueType::INT},
		{TokenType::FLOAT_LITERAL, LValueType::FLOAT}
	};

	struct AST_Literal_Expression : Expression {

		string value;

		AST_Literal_Expression() {
			expressionType = ExpressionType::LITERAL;
			isPointer = false;
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