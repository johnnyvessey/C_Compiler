#include "Parser.h"


AST::AST(vector<Token>& tokens) : tokens(tokens)
{
	group = make_unique<StatementGroup>();
}



unique_ptr<Statement> AST::ParseStatement()
{
	Token& currentToken = tokens.at(currentIndex);
	switch (currentToken.type)
	{
		//TODO: figure out how to handle weird statements that don't mean anything but have side effects
		//			such as 5 + (++x); or 5 + f(x) [where f() has side effects]
	case TokenType::NEW_LINE:
	{
		++currentIndex;
		return nullptr;
	}
	case TokenType::RETURN:
	{
		AST_Return_Statement retStatement;
		++currentIndex;
		if (GetCurrentToken().type == TokenType::SEMICOLON)
		{
			retStatement.returnExpression = nullptr;
		}
		else {
			retStatement.returnExpression = ParseExpression();
			assert(GetCurrentToken().type == TokenType::SEMICOLON, "Statement must end with semicolon", GetCurrentLineNum());
		}
		++currentIndex;
		return make_unique<AST_Return_Statement>(std::move(retStatement));
	}
	case TokenType::TYPE:
	{
		if (tokens.at(currentIndex + 1).type == TokenType::NAME && tokens.at(currentIndex + 2).type == TokenType::OPEN_PAR)
		{
			return std::move(ParseFunctionDefinition());
		}
		else 
		{
			return make_unique<AST_Assignment>(std::move(ParseInitAssignment()));
		}
	}
	case TokenType::STRUCT:
	{
		if (tokens.at(currentIndex + 1).type == TokenType::NAME && tokens.at(currentIndex + 2).type == TokenType::OPEN_BRACE)
		{
			return make_unique<AST_Struct_Definition>(std::move(ParseStructDefinition()));
		}
		else if (tokens.at(currentIndex + 1).type == TokenType::NAME && tokens.at(currentIndex + 2).type == TokenType::NAME)
		{
			++currentIndex;
			if (tokens.at(currentIndex + 2).type == TokenType::OPEN_PAR)
			{
				return std::move(ParseFunctionDefinition());
			}
			else 
			{
				return make_unique<AST_Assignment>(std::move(ParseInitAssignment()));
			}
		}
		else {
			throwError("Invalid syntax", currentToken.lineNumber);
		}
			
		}
		case TokenType::NAME:
		{
			Token& nextToken = tokens.at(currentIndex + 1);
			if (Lexer::IsAssignmentOp(nextToken.type)) // =, +=, -=, *=, /=, %=
			{
				return make_unique<AST_Assignment>(std::move(ParseAssignment()));
			}
			else
			{
				//also includes function calls
				unique_ptr<Expression> expr = ParseExpression();
				AST_Expression_Statement statement;
				statement.expr = std::move(expr);
				assert(GetCurrentToken().type == TokenType::SEMICOLON, "Statement must end with semicolon", GetCurrentLineNum());
				++currentIndex;
				return make_unique<AST_Expression_Statement>(std::move(statement));
			}
		}
		case TokenType::IF:
		{
			return ParseIfStatement();
		}
		default:
			AST_Expression_Statement statement;
			statement.expr = ParseExpression();
			assert(GetCurrentToken().type == TokenType::SEMICOLON, "Statement must end with semicolon", GetCurrentLineNum());
			++currentIndex;
			return make_unique<AST_Expression_Statement>(std::move(statement));
	}
		
}

void AST::ParseProgram()
{
	while (tokens.at(currentIndex).type != TokenType::END_OF_FILE)
	{
		unique_ptr<Statement> statement = ParseStatement();
		if (statement)
		{
			group->statements.push_back(std::move(statement));
		}
	}
}




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
