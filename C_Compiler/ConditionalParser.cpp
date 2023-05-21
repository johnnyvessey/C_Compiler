#include "Parser.h"

void AST::ParseIfStatement(unique_ptr<AST_If_Then>& ifThenExpr, unique_ptr<Expression>& condition, unique_ptr<StatementGroup>& group)
{
	assert(tokens.at(currentIndex + 1).type == TokenType::OPEN_PAR, "If statement requires parentheses", tokens.at(currentIndex + 1).lineNumber);
	currentIndex += 2;
	condition = std::move(ParseParentheticalExpression());
	assert(IsNumericType(condition->type), "Condition must be scalar type", tokens.at(currentIndex).lineNumber);

	ParseConditionalSubstatements(group);
}

void AST::ParseElseStatement(unique_ptr<StatementGroup>& group)
{
	++currentIndex;
	ParseConditionalSubstatements(group);
}

void AST::ParseConditionalSubstatements(unique_ptr<StatementGroup>& group)
{
	assert(GetCurrentToken().type == TokenType::OPEN_BRACE, "Open brace required for if statement", GetCurrentLineNum());
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
	ParseIfStatement(ifThenExpr, ifThenExpr->Condition, ifThenExpr->ifStatement);

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