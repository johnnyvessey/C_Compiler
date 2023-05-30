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
using std::shared_ptr;
using std::make_unique;


using namespace AST_Expression;
using namespace VariableNamespace;

namespace AST_Statement 
{
	
	struct Statement {
		 virtual void PrintStatementAST(int indentLevel = 0) = 0;
	};


	struct StatementGroup : Statement {
		vector<unique_ptr<Statement>> statements;

		virtual void PrintStatementAST(int indentLevel = 0) override;
	};

	struct AST_Initialization : Statement {
		unique_ptr<Variable> lvalue;
		unique_ptr<Expression> rvalue;

		virtual void PrintStatementAST(int indentLevel = 0) override;
	};


	struct AST_Assignment : Statement {
		unique_ptr<LValueExpression> lvalue;
		unique_ptr<Expression> rvalue;
		TokenType assignmentOperator;

		virtual void PrintStatementAST(int indentLevel = 0) override;
	};
	

	struct AST_Else_If : Statement {
		unique_ptr<Expression> condition;
		unique_ptr<StatementGroup> statements;

		AST_Else_If();

		virtual void PrintStatementAST(int indentLevel = 0) override;
	};


	struct AST_If_Then : Statement {
		unique_ptr<Expression> Condition;
		unique_ptr<StatementGroup> ifStatement;
		vector<unique_ptr<AST_Else_If>> elseIfStatements;
		unique_ptr<StatementGroup> elseStatement;

		AST_If_Then();
		virtual void PrintStatementAST(int indentLevel = 0) override;
	};



	struct AST_Expression_Statement : Statement
	{
		unique_ptr<Expression> expr;

		virtual void PrintStatementAST(int indentLevel = 0) override;
	};



	struct Function {
		string name;
		vector<Variable> arguments;
		shared_ptr<StatementGroup> statements;
		VariableType returnType;
		string returnTypeStructName;
	};

	struct AST_Function_Definition : Statement
	{
		unique_ptr<Function> func;

		virtual void PrintStatementAST(int indentLevel = 0) override;

	};


	struct AST_Struct_Definition : Statement {
		string name;
		unordered_map<string, Struct_Variable> variableMapping;
		size_t memorySize;

		AST_Struct_Definition(string name, vector<Struct_Variable>&& variables, size_t memorySize);
		AST_Struct_Definition();

		virtual void PrintStatementAST(int indentLevel = 0) override;
	};

	struct AST_Return_Statement : Statement {
		unique_ptr<Expression> returnExpression;

		virtual void PrintStatementAST(int indentLevel = 0) override;
	};

	//struct AST_While_Loop : Statement {
	//	//Expression condition;
	//	unique_ptr<StatementGroup> statements;
	//};
	//	
}