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

using namespace VariableNamespace;

namespace AST_Expression {
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

	inline bool IsBooleanOperation(BinOpType type)
	{
		return type == EQUALS || type == NOT_EQUALS || type == LESS || type == LESS_EQUAL || type == GREATER || type == GREATER_EQUAL || type == AND || type == OR;
	}

	struct BinOp {
		BinOpType type;
		int precedence;

		BinOp(BinOpType type, int precedence);
	};

	struct ExpressionUtils
	{
		static unordered_map<TokenType, BinOp> BinOpTokenDictionary;
		static unordered_map<TokenType, BinOpType> BinOpAssignmentTypeDictionary;
		static unordered_map<TokenType, LValueType> TokenTypeToLValueTypeMapping;

	};


	struct Expression {
		VariableType type;

		ExpressionType expressionType;

		bool isLValue = false;

		Expression();

		virtual void PrintExpressionAST(int indentLevel = 0) = 0;
	};

	struct LValueExpression : Expression
	{

	};

	struct AST_BinOp : Expression {
		BinOpType op;
		unique_ptr<Expression> left;
		unique_ptr<Expression> right;

		AST_BinOp();

		virtual void PrintExpressionAST(int indentLevel = 0) override;
	};


	struct AST_Type_Cast_Expression : Expression
	{
		//figure out pointer casting!! and how to represent pointers. Maybe have int count of number of pointers (i.e: value of 3 for int*** x;)
		LValueType from;
		LValueType to;
		virtual void PrintExpressionAST(int indentLevel = 0) override;
	};


	struct AST_Function_Expression : Expression
	{
		vector<unique_ptr<Expression>> argumentInstances;
		string functionName;

		AST_Function_Expression();

		virtual void PrintExpressionAST(int indentLevel = 0) override;
	};


	struct BinOperator {
		BinOpType type;
	};


	struct AST_Variable_Expression : LValueExpression
	{
		Variable v;
		virtual void PrintExpressionAST(int indentLevel = 0) override;


		AST_Variable_Expression(Variable v);
	};


	struct AST_Literal_Expression : Expression {

		string value;

		AST_Literal_Expression();

		virtual void PrintExpressionAST(int indentLevel = 0) override;
	};


	struct AST_Struct_Variable_Access : LValueExpression {

		unique_ptr<Expression> expr;
		string varName;

		AST_Struct_Variable_Access();
		virtual void PrintExpressionAST(int indentLevel = 0) override;
	};


	struct AST_Pointer_Dereference : LValueExpression
	{
		unique_ptr<Expression> baseExpr;

		AST_Pointer_Dereference();

		virtual void PrintExpressionAST(int indentLevel = 0) override;
	};

	struct AST_Variable_Reference : Expression
	{
		unique_ptr<LValueExpression> expr;

		virtual void PrintExpressionAST(int indentLevel = 0) override;
	};

	struct AST_Unary_Assignment_Expression : Expression
	{
		unique_ptr<LValueExpression> expr;
		bool isPrefix;
		//unary op type type
		TokenType opType;

		virtual void PrintExpressionAST(int indentLevel = 0) override;
	};

	struct AST_Negative_Expression : Expression //negative, not, reference
	{
		unique_ptr<Expression> expr;

		virtual void PrintExpressionAST(int indentLevel = 0) override;
	};

	struct AST_Address_Expression : Expression
	{
		unique_ptr<Expression> expr;

		virtual void PrintExpressionAST(int indentLevel = 0) override;
	};

	struct AST_Not_Expression : Expression
	{
		unique_ptr<Expression> expr;

		virtual void PrintExpressionAST(int indentLevel = 0) override;
	};

}