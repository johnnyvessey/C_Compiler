#pragma once

#include <unordered_map>;
#include <vector>
#include <string>
#include "Lexer.h"
#include <algorithm>
#include <memory>
#include <iostream>

using std::unordered_map;
using std::vector;
using std::string;
using std::unique_ptr;
using std::make_unique;


using namespace Lexer;

namespace AST_Definitions {

	enum LValueType {
		INT,
		FLOAT,
		STRUCT
	};

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

	//PROBABLY JUST USE LVALUE TYPE (BUT ASSUME IT CAN'T BE STRUCT TYPE FOR LITERALS)
	//enum LiteralType {
	//	INT_LITERAL_TYPE,
	//	FLOAT_LITERAL_TYPE
	//};


	struct Variable {
		string name;
		LValueType type;
		string structName;
		bool isPointer;
	};

	struct Function {
		string name;
		vector<LValueType> argumentTypes;
		LValueType returnType;
	};


	struct Statement {
		 virtual void PrintStatementAST() = 0;
	};


	struct StatementGroup : Statement {
		vector<unique_ptr<Statement>> statements;

		virtual void PrintStatementAST() override;
	};

	void StatementGroup::PrintStatementAST()
	{
		for (const unique_ptr<Statement>& statement : statements)
		{
			statement->PrintStatementAST();
			std::cout << "\n-----------------------\n";
		}
	}

	struct Expression {
		LValueType type;
		string structName;
		bool isPointer;
		ExpressionType expressionType;

		Expression()
		{
			expressionType = ExpressionType::NONE;
		}

		virtual void PrintExpressionAST() = 0;
	};

	void Expression::PrintExpressionAST()
	{
		std::cout << "Error: unkown expression type.\n";
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


	struct AST_BinOp : Expression {
		BinOpType op;
		unique_ptr<Expression> left;
		unique_ptr<Expression> right;

		AST_BinOp() {
			expressionType = ExpressionType::BINARY_OPERATION;
		}

		virtual void PrintExpressionAST() override;
	};

	void AST_BinOp::PrintExpressionAST() {

	}

	struct AST_Function_Expression : Expression
	{

		AST_Function_Expression() {
			expressionType = ExpressionType::FUNCTION_CALL;
		}

		virtual void PrintExpressionAST() override;
	};

	void AST_Function_Expression::PrintExpressionAST() {

	}


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

		virtual void PrintExpressionAST() override;
	};

	void AST_Literal_Expression::PrintExpressionAST()
	{
		std::cout << "Literal: " << value << "\n";
	}

	struct AST_Assignment : Statement {
		unique_ptr<Variable> lvalue;
		unique_ptr<Expression> rvalue;

		virtual void PrintStatementAST() override;
	};

	void AST_Assignment::PrintStatementAST()
	{
		std::cout << "Assign: " << lvalue->name << "; type: " << lvalue->type << " " << lvalue->structName << "\n\t";
		rvalue->PrintExpressionAST();
	}

	//struct AST_If_Then : Statement {
	//	Expression* Condition;
	//	StatementGroup* ifStatement;
	//	//vector<Statement> elseIfStatments;
	//	StatementGroup* elseStatement;
	//};


	struct Struct_Variable {
		Variable v;
		size_t memoryOffset;
	};

	size_t GetMemoryFromType(LValueType type, string structName = "")
	{
		if (type == LValueType::INT || type == LValueType::FLOAT)
		{
			return 4;
		}
		else if (type == LValueType::STRUCT)
		{
			return 0; //TODO: FIGURE OUT HOW TO GET REFERENCE TO SPECIFIC STRUCT
		}
	}


	struct AST_Struct_Definition : Statement {
		string name;
		unordered_map<string, Struct_Variable> variableMapping;
		size_t memorySize;

		AST_Struct_Definition(string name, vector<Struct_Variable>&& variables) : name(name)
		{
			size_t currentOffset = 0;
			for (auto&& var : variables)
			{
				var.memoryOffset = currentOffset;
				variableMapping[var.v.name] = var;

				currentOffset += GetMemoryFromType(var.v.type, var.v.structName);
			}

			memorySize = currentOffset; //figure out if I need to align struct (to 4 bytes, for example)
		}

		virtual void PrintStatementAST() override;
	};

	void AST_Struct_Definition::PrintStatementAST()
	{

	}

	//struct AST_While_Loop : Statement {
	//	//Expression condition;
	//	unique_ptr<StatementGroup> statements;
	//};
	//


	/*
	SCOPE of variables:
	- vector functioning as stack
	- push new scope object to stack when entering, pop when leaving scope (don't care about variables once we're out of scope)
	- iterate through vector backwards to find the variable/function/struct name
	*/
	struct ScopeLevel {
		unordered_map<string, Variable> variables;
		unordered_map<string, Function> functions;
		unordered_map<string, AST_Struct_Definition> structs;
	};

	struct Scope {
		vector<ScopeLevel> scope;

		Scope() {
			scope = vector<ScopeLevel>(1, ScopeLevel());
		}
		bool TryFindVariable(const string& name, Variable& var)
		{
			for (int i = scope.size() - 1; i >= 0; --i)
			{
				if (scope[i].variables.find(name) != scope[i].variables.end())
				{
					var = scope[i].variables.at(name);
					return true;
				}
			}

			return false;
		}

		bool TryFindFunction(const string& name, Function& func)
		{
			for (int i = scope.size() - 1; i >= 0; --i)
			{
				if (scope[i].functions.find(name) != scope[i].functions.end())
				{
					func = scope[i].functions.at(name);
					return true;
				}
			}

			return false;
		}

		bool TryFindStructName(const string& name, AST_Struct_Definition& structDef)
		{
			for (int i = scope.size() - 1; i >= 0; --i)
			{
				if (scope[i].structs.find(name) != scope[i].structs.end())
				{
					structDef = scope[i].structs.at(name);
					return true;
				}
			}

			return false;
		}
	};


}