#include "Parser.h"

void AST::ParseIfStatement(unique_ptr<AST_If_Then>& ifThenExpr, unique_ptr<Expression>& condition, unique_ptr<StatementGroup>& group)
{
	assert(tokens.at(currentIndex + 1).type == TokenType::OPEN_PAR, "If statement requires parentheses", tokens.at(currentIndex + 1).lineNumber);
	currentIndex += 2;
	condition = std::move(ParseParentheticalExpression());
	assert(IsNumericType(condition->type.lValueType), "Condition must be scalar type", tokens.at(currentIndex).lineNumber);

	ParseScopeStatements(group);
}

void AST::ParseElseStatement(unique_ptr<StatementGroup>& group)
{
	++currentIndex;
	ParseScopeStatements(group);
}

void AST::ParseScopeStatements(unique_ptr<StatementGroup>& group)
{
	assert(GetCurrentToken().type == TokenType::OPEN_BRACE, "Open brace required for scope statement", GetCurrentLineNum());
	++currentIndex;

	scopeStack.scope.push_back(std::move(ScopeLevel()));
	//TODO: figure out if this could ever be infinite loop
	while (GetCurrentToken().type != TokenType::CLOSE_BRACE)
	{
		group->statements.push_back(std::move(ParseStatement()));
	}
	scopeStack.scope.pop_back();
	++currentIndex;
}


unique_ptr<AST_If_Then> AST::ParseIfStatement()
{
	unique_ptr<AST_If_Then> ifThenExpr = make_unique<AST_If_Then>(std::move(AST_If_Then()));
	ParseIfStatement(ifThenExpr, ifThenExpr->condition, ifThenExpr->ifStatement);

	//parse chain of else ifs

	while (tokens.at(currentIndex).type == TokenType::ELSE && tokens.at(currentIndex + 1).type == TokenType::IF)
	{
		++currentIndex;
		unique_ptr<AST_Else_If> elseIf = make_unique<AST_Else_If>();
		ParseIfStatement(ifThenExpr, elseIf->condition, elseIf->statements);
		ifThenExpr->elseIfStatements.push_back(std::move(elseIf));
	}

	if (GetCurrentToken().type == TokenType::ELSE)
	{
		ParseElseStatement(ifThenExpr->elseStatement);
	}

	return ifThenExpr;

}


unique_ptr<AST_While_Loop> AST::ParseWhileLoop()
{
	unique_ptr<AST_While_Loop> whileLoop = make_unique<AST_While_Loop>();

	++currentIndex;

	whileLoop->Condition = ParseExpression();

	assert(whileLoop->Condition->type.lValueType == LValueType::INT, "While loop condition must be boolean", GetCurrentLineNum());

	whileLoop->Statements = make_unique<StatementGroup>();
	ParseScopeStatements(whileLoop->Statements);

	return whileLoop;
}

unique_ptr<AST_For_Loop> AST::ParseForLoop()
{
	unique_ptr<AST_For_Loop> forLoop = make_unique<AST_For_Loop>();

	++currentIndex;
	assert(GetCurrentToken().type == TokenType::OPEN_PAR, "For loop must start with open parentheses", GetCurrentLineNum());
	++currentIndex;

	forLoop->First = ParseSingleStatement();
	if (GetCurrentToken().type == TokenType::SEMICOLON)
	{
		unique_ptr<AST_Literal_Expression> trueExpr = make_unique<AST_Literal_Expression>();
		trueExpr->value = "1";

		forLoop->Condition = std::move(trueExpr);
	}
	else {
		forLoop->Condition = ParseExpression();
		assert(forLoop->Condition->type.lValueType == LValueType::INT, "Second for-loop expression must be boolean", GetCurrentLineNum());
	}

	assert(GetCurrentToken().type == TokenType::SEMICOLON, "Second for-loop expression must end with semicolon", GetCurrentLineNum());
	++currentIndex;

	
	forLoop->Third = (GetCurrentToken().type == TokenType::CLOSE_PAR) ? nullptr : ParseExpression();
	assert(GetCurrentToken().type == TokenType::CLOSE_PAR, "For loop must end with close parentheses", GetCurrentLineNum());

	++currentIndex;
	forLoop->Statements = make_unique<StatementGroup>();
	ParseScopeStatements(forLoop->Statements);

	return forLoop;
}