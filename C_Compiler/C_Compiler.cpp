// C_Compiler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

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

using namespace Lexer;
using namespace AST_Expression;
using namespace AST_Statement;
using namespace ScopeNamespace;


void assert(bool condition, string message, size_t lineNum)
{
	if (!condition)
	{
		std::cout << "ERROR: " << message << "; at line: " << lineNum;
		throw 0;
	}
}

void throwError(string message, size_t lineNum)
{
	assert(false, message, lineNum);
}


//struct LValue {
//	LValueType type;
//	string name;
//	string structName;
//};

//struct AssignmentOperator {};




class AST
{
public:
	vector<Token> tokens;
	unique_ptr<StatementGroup> group;

	size_t currentIndex = 0;
	Scope scopeStack;	

	AST(vector<Token>& tokens) : tokens(tokens)
	{
		group = make_unique<StatementGroup>();
	}

	//Expression ParseCondition()
	//{

	//}

	//AST_If_Then ParseIfThen()
	//{
	//	AST_If_Then ifThen;
	//	ifThen.Condition = ParseCondition();

	//	++scopeLevel;
	//	ifThen.ifStatement = ParseStatement();
	//	--scopeLevel;

	//	if (tokens.at(currentIndex).type == TokenType::ELSE)
	//	{
	//		++scopeLevel;
	//		ifThen.elseStatement = ParseStatement();
	//		--scopeLevel;
	//	}

	//}

	 
	LValueType GetTypeFromName(string&& name)
	{
		if (name == "int")
		{
			return LValueType::INT;
		}
		else if (name == "float")
		{
			return LValueType::FLOAT;
		}
		else {
			return LValueType::STRUCT;
		}
	}

	/*unique_ptr<AST_BinOp> ParseRightHandBinaryOperation(unique)
	{
		Token token = tokens.at(currentIndex);
		Token nextToken = tokens.at(currentIndex + 1);

		unique_ptr<AST_BinOp> binOpExpr = make_unique<AST_BinOp>(AST_BinOp());
		binOpExpr->op = BinOpDict.at(nextToken.type);

		AST_Literal_Expression left;
		left.literalType = TokenTypeToLiteralTypeMapping.at(token.type);
		left.value = token.value;
		binOpExpr->left = make_unique<AST_Literal_Expression>(std::move(left));

		currentIndex += 2;
		binOpExpr->right = make_unique<AST_Literal_Expression>(std::move(ParseExpression()));

		assert(binOpExpr->left->type == binOpExpr->right->type, "Binary operands' types do not match up on line: " + token.lineNumber);

		return binOpExpr;
	}*/
	unique_ptr<Expression> ParseParentheticalExpression()
	{
		Token& token = tokens.at(currentIndex);
		unique_ptr<Expression> parExpr;

		if (token.type == TokenType::CLOSE_PAR)
		{
			assert(parExpr->expressionType != ExpressionType::NONE, "Invalid parenthetical expression", token.lineNumber);
			++currentIndex;
			return parExpr;
		}
		else if (token.type == TokenType::INT_LITERAL || token.type == TokenType::FLOAT_LITERAL)
		{
			AST_Literal_Expression litExpr;
			litExpr.value = token.value;
			litExpr.type = TokenTypeToLValueTypeMapping.at(token.type);

			Token& nextToken = tokens.at(currentIndex + 1);

			if (nextToken.type == TokenType::CLOSE_PAR)
			{
				currentIndex += 2;
				return make_unique<AST_Literal_Expression>(std::move(litExpr));
			}
			else if (IsBinOp(nextToken.type))
			{
				unique_ptr<AST_BinOp> binOpExpr = make_unique<AST_BinOp>();
				binOpExpr->op = BinOpDict.at(nextToken.type);

				//binOpExpr->left = make_unique<AST_Literal_Expression>(std::move(litExpr));

				//currentIndex += 2;
				////TODO: Figure out how to handle operator precedence!!!
				////binOpExpr->right = make_unique<AST_Literal_Expression>(std::move(ParseExpression()));

				//assert(binOpExpr->left->type == binOpExpr->right->type, "Binary operands' types do not match up", token.lineNumber);

				return binOpExpr;
			}
		}
	}
	unique_ptr<Expression> ParseExpression()
	{
		unique_ptr<Expression> expr;
		Token& token = tokens.at(currentIndex);
		Token& nextToken = tokens.at(currentIndex + 1);

		if (token.type == TokenType::OPEN_PAR)
		{
			++currentIndex;
			expr = ParseParentheticalExpression();
		}
		else if (token.type == TokenType::NAME)
		{
			//FIND IF FUNCTION
			// 
			//FIND IF VARIABLE
			if (nextToken.type == OPEN_PAR) //function
			{
				Function f;
				bool foundFunction = scopeStack.TryFindFunction(token.value, f);
				assert(foundFunction, "Function: " + token.value + " doesn't exist in scope", token.lineNumber);

			}
			else //variable
			{ 
				Variable v;
				bool foundVariable = scopeStack.TryFindVariable(token.value, v);
				assert(foundVariable, "Variable: " + token.value + " doesn't exist in scope", token.lineNumber);

				if (nextToken.type == TokenType::PERIOD)  //struct variable access
				{

				}
				else if (IsBinOp(nextToken.type)) //binary operation
				{

				}
				else if (IsUnaryOp(nextToken.type)) //unary operation
				{

				}
				else if (nextToken.type == TokenType::ARROW) //dereference struct pointer
				{

				}
				else {
					assert(false, "Invalid syntax for expression", nextToken.lineNumber);
				}
			}
		}
		else if (token.type == TokenType::INT_LITERAL || token.type == TokenType::FLOAT_LITERAL)
		{

			if (nextToken.type == TokenType::SEMICOLON || nextToken.type == TokenType::COMMA || nextToken.type == TokenType::CLOSE_PAR)
			{
				currentIndex += (nextToken.type == TokenType::SEMICOLON ? 2 : 1); //if semicolon, continue to next statement. Otherwise, let comma/close_par be parsed after return
				AST_Literal_Expression expr;

				//expand this line to fit other literal types??
				expr.type = token.type == TokenType::INT_LITERAL ? LValueType::INT : LValueType::FLOAT;
				expr.value = token.value;
				expr.isPointer = false;

				return make_unique<AST_Literal_Expression>(std::move(expr));;
			}
			else if (IsBinOp(nextToken.type))
			{

			}
			else {
				throwError("Invalid syntax", token.lineNumber);
			}
		}
		else if (token.type == TokenType::STAR) //pointer dereferencing
		{

		}

		return expr; //CHANGE THIS!!!!
	}

	AST_Assignment ParseInitAssignment()
	{
		//TODO: Include function assignment [REMOVE ABILITY TO DECLARE VARIABLES LIKE int x(2);]
		// Don't create variable yet until you know it's a variable assignment and not a function one
		//int x = EXPR
		AST_Assignment assignment;
		Variable v;
		v.type = GetTypeFromName(std::move(tokens.at(currentIndex).value));
		Token& nextToken = tokens.at(currentIndex + 1);
		if (nextToken.type == TokenType::NAME) {
			v.name = tokens.at(currentIndex + 1).value;
			v.isPointer = false;

			currentIndex += 2;
		}
		else if (nextToken.type == TokenType::STAR)
		{
			v.name = tokens.at(currentIndex + 2).value;
			v.isPointer = true;

			currentIndex += 3;
		}
		else {
			throwError("Improper variable assignment", nextToken.lineNumber);
		}
		assignment.lvalue = make_unique<Variable>(v);

		//TODO: ADD VARIABLE TO SCOPE
		//TODO: FIGURE OUT HOW TO PARSE ARRAY

		if (tokens.at(currentIndex).type == TokenType::SINGLE_EQUAL)
		{
			++currentIndex;
			unique_ptr<Expression> expr = ParseExpression();
			assert(expr->type == v.type && expr->structName == v.structName && v.isPointer == expr->isPointer, "Variable is assigned to the wrong type", tokens.at(currentIndex).lineNumber);
			assignment.rvalue = std::move(expr);
		}

		assert(tokens.at(currentIndex).type == TokenType::SEMICOLON, "Semicolon required at the end of this line", tokens.at(currentIndex).lineNumber);
		++currentIndex;
		
		return assignment;
	}

	AST_Struct_Definition ParseStructDefinition()
	{
		assert(tokens.at(currentIndex + 2).type == TokenType::OPEN_BRACE, "Struct definition requires open brace", tokens.at(currentIndex).lineNumber);
		vector<Struct_Variable> structVariables;

		//TODO: FINISH THIS

		return AST_Struct_Definition("NO NAME", vector<Struct_Variable>());
	}

	//Statement ParseStructVariableInitialization()
	//{

	//}

	void ParseStatement()
	{
		Token& currentToken = tokens.at(currentIndex);
		switch (currentToken.type)
		{
		case TokenType::NEW_LINE:
			++currentIndex;
			return;
		case TokenType::TYPE:
			group->statements.push_back(make_unique<AST_Assignment>(ParseInitAssignment()));
			break;
		case TokenType::STRUCT:
			if (tokens.at(currentIndex + 1).type == TokenType::NAME && tokens.at(currentIndex + 2).type == TokenType::OPEN_BRACE)
			{
				group->statements.push_back(make_unique<AST_Struct_Definition>(std::move(ParseStructDefinition())));
			}
			else {
				group->statements.push_back(make_unique<AST_Struct_Definition>(std::move(ParseStructDefinition())));
			}

			break;

		case TokenType::NAME:

			break;
		}
		//else if (currentToken.type == TokenType::IF) {
		//	group.statements->push_back(ParseIfThen());
		//}
	}

	void ParseProgram()
	{
		while (tokens.at(currentIndex).type != TokenType::END_OF_FILE)
		{
			ParseStatement();
		}
	}

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
int main()
{
	const string input = "int x = 5;";
	auto tokens = SplitStringByToken(input);
	std::cout << input << "\n\n";
	for (auto& s : tokens) {
		std::cout << s.type << ": " << Lexer::GetNameFromEnum(s.type) << ": " << s.value << " - line: " << s.lineNumber << " - token: " << s.tokenNumber << "\n";
	}

	AST ast(tokens);
	ast.ParseProgram();
	ast.group->PrintStatementAST();
}


//have state object (representing current scope, defined variables / structs, etc...
//iterate through vector of strings
 

