#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "Lexer.h"
#include "AST_Statement.h"
#include "Scope.h"

using std::string;
using std::vector;
using std::unordered_map;
using std::unique_ptr;
using std::make_unique;

using namespace AST_Expression;
using namespace AST_Statement;

inline void assert(bool condition, string message, size_t lineNum)
{
	if (!condition)
	{
		std::cout << "ERROR: " << message << "; at line: " << lineNum;
		throw 0;
	}
}

inline void throwError(string message, size_t lineNum)
{
	assert(false, message, lineNum);
}

class AST
{
public:
	vector<Token> tokens;
	unique_ptr<StatementGroup> group;

	size_t currentIndex = 0;
	Scope scopeStack;

	AST(vector<Token>& tokens);

	size_t GetCurrentLineNum();

	Token& GetCurrentToken();

	int GetConsecutiveTokenNumber(TokenType type);

	LValueType GetTypeFromName(string&& name);

	unique_ptr<AST_BinOp> ParseBinaryExpression(unique_ptr<Expression> firstExpr);

	vector<Variable> ParseFunctionParameters();

	unique_ptr<AST_Function_Definition> ParseFunctionDefinition();

	unique_ptr<Expression> ParseNonBinaryExpression();

	unique_ptr<Expression> ParseParentheticalExpression();

	unique_ptr<Expression> ParseExpression();

	AST_Assignment ParseInitAssignment();

	//TODO: Figure out if assignment and initialization should be different in AST
	AST_Assignment ParseAssignment();

	AST_Struct_Definition ParseStructDefinition();

	void ParseConditionalSubstatements(unique_ptr<StatementGroup>& group);

	void ParseIfStatement(unique_ptr<AST_If_Then>& ifThenExpr, unique_ptr<Expression>& condition, unique_ptr<StatementGroup>& group);

	void ParseElseStatement(unique_ptr<StatementGroup>& group);

	unique_ptr<Statement> ParseStatement();

	void ParseProgram();

	unique_ptr<AST_If_Then> ParseIfStatement();
};


/*
	IF:
		Parse: if, open par, Expression (assert type is bool), close par, open brace:
			Keep parsing statements until get to close brace of start scope (push to "if" statement group)
			If else shows up after close brace, repeat same process (push to "else" statement group)
	WHILE:
		Parse: while, open par, Expression (assert type is bool), close par, open brace
		->repeat same process as if

----
	General Expression Parsing Algorithm:

	name: check if variable or function
		if function: parse function expression (shouldn't be too hard... hopefully), then continue
		if variable: create expression from variable, then keep going to see if I should put it in bin op expression
			check if next token is period / -> operator to see whether to access struct variable
	literal:
		either return it directly or put in in bin op
	parentheses:
		handle it separately
	star:
		dereference pointer


----

x + y
	Parsing Expressions Examples:
	[DON'T ALLOW C STYLE CASTS -> have predefined cast function]

	Single Var:
		5
		x
	Negative Values: (currently - is its own token)
		-5
		-x
	Bin Op:
		5 + 6
	Mult Bin Ops:
		1 + 2 + 3
	Parentheses:
		(1 + 2) * 3
	Variable in Operation:
		x + 2
	Multiple Parentheses:
		((1 + 3) * 2) + x
	Function calls:
		f(x) + 2
	Struct variables:
		structObj.var * 2
	Pointer dereferencing:
		*ptr + 4
	Pointer dereferencing on struct:
		structPtr->var * 2
		*(structPtr).var * 2 [Maybe I can leave this option out, and force the -> operator]
	Pointer dereferencing with multiplication:
		*ptr * x [this will be very hard to parse!!!]
		4 * ptr [throw exception, since ptr is a pointerType]


		(5 + 7) * (*ptr) + (1 + (1 * 2))

		5 + x * 7


	Parsing function (no function overloading allowed):
		Look up name in scope tree
		If not found, throw exception
		Parse open par, Parse expression (assert type equals the function type), parse comma, (repeated)...,  until matching end par
			Since there is no function overloading, use for loop with explicitly known number of arguments
			ex:

			FunctionExpression f_expr;
			for(size_t i = 0; i < function.numArgs; ++i)
			{
				Expression expr = ParseExpression();
				expr.type = function.returnType;
				assert(expr.type == function.args[i].type, "Wrong function type");
				if( i < function.numArgs - 1) {
				assert(tokens.at(currentIndex).type == TokenType::COMMA, "No comma);
				++currentIndex;

				f_expr.argExprs.push_back(expr);
				}
			}
			assert(tokens.at(currentIndex).type == TokenType::CLOSE_PAR, "No close par);
			++currentIndex;

			return f_expr;


*/

