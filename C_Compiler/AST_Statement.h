#pragma once

#include <unordered_map>;
#include <vector>
#include <string>
#include "Lexer.h"
#include <algorithm>
#include <memory>
#include <iostream>
#include "AST_Expression.h"
#include "Variable.h"

using std::unordered_map;
using std::vector;
using std::string;
using std::unique_ptr;
using std::make_unique;


using namespace Lexer;
using namespace AST_Expression;
using namespace VariableNamespace;

namespace AST_Statement {

	//PROBABLY JUST USE LVALUE TYPE (BUT ASSUME IT CAN'T BE STRUCT TYPE FOR LITERALS)
	//enum LiteralType {
	//	INT_LITERAL_TYPE,
	//	FLOAT_LITERAL_TYPE
	//};


	struct Function {
		string name;
		vector<LValueType> argumentTypes;
		LValueType returnType;
	};


	struct Statement {
		 virtual void PrintStatementAST(int indentLevel = 0) = 0;
	};


	struct StatementGroup : Statement {
		vector<unique_ptr<Statement>> statements;

		virtual void PrintStatementAST(int indentLevel = 0) override;
	};

	void StatementGroup::PrintStatementAST(int indentLevel)
	{
		for (const unique_ptr<Statement>& statement : statements)
		{
			statement->PrintStatementAST(indentLevel);
			std::cout << "\n-----------------------\n";
		}
	}


	struct AST_Assignment : Statement {
		unique_ptr<Variable> lvalue;
		unique_ptr<Expression> rvalue;
		bool isInitialization;

		virtual void PrintStatementAST(int indentLevel = 0) override;
	};

	void AST_Assignment::PrintStatementAST(int indentLevel)
	{
		std::cout << string(indentLevel, '\t') << "Assign: " << lvalue->name << "; type: " << lvalue->type << " " << lvalue->structName << "\n";
		rvalue->PrintExpressionAST(indentLevel + 1);
	}

	struct AST_Else_If : Statement {
		unique_ptr<Expression> condition;
		unique_ptr<StatementGroup> statements;

		AST_Else_If()
		{
			statements = make_unique<StatementGroup>();
		}
		virtual void PrintStatementAST(int indentLevel = 0) override;
	};

	void AST_Else_If::PrintStatementAST(int indentLevel)
	{
		std::cout << string(indentLevel, '\t') << "else if: \n";
		condition->PrintExpressionAST(indentLevel);
		statements->PrintStatementAST(indentLevel + 2);
	}


	struct AST_If_Then : Statement {
		unique_ptr<Expression> Condition;
		unique_ptr<StatementGroup> ifStatement;
		vector<unique_ptr<AST_Else_If>> elseIfStatements;
		unique_ptr<StatementGroup> elseStatement;

		AST_If_Then()
		{
			ifStatement = make_unique<StatementGroup>();
			elseStatement = make_unique<StatementGroup>();
		}
		virtual void PrintStatementAST(int indentLevel = 0) override;
	};

	void AST_If_Then::PrintStatementAST(int indentLevel)
	{
		std::cout << string(indentLevel, '\t') << "if statement:\n";
		Condition->PrintExpressionAST(indentLevel);
		std::cout << string(indentLevel + 1, '\t') << "then:\n";
		ifStatement->PrintStatementAST(indentLevel + 2);

		for (const auto& elseIfStatement : elseIfStatements)
		{
			elseIfStatement->PrintStatementAST(indentLevel + 1);
		}
		std::cout << string(indentLevel + 1, '\t') << "else:\n";
		elseStatement->PrintStatementAST(indentLevel + 2);
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

		virtual void PrintStatementAST(int indentLevel = 0) override;
	};

	void AST_Struct_Definition::PrintStatementAST(int indentLevel)
	{

	}

	//struct AST_While_Loop : Statement {
	//	//Expression condition;
	//	unique_ptr<StatementGroup> statements;
	//};
	//	


}