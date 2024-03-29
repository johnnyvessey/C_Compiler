#include "Parser.h"


int AST::GetCurrentLineNum()
{
	return tokens.at(currentIndex).lineNumber;
}

Token& AST::GetCurrentToken()
{
	return tokens.at(currentIndex);
}


/*void SkipNewLines()
{
	while (tokens.at(currentIndex).type == TokenType::NEW_LINE)
	{
		++currentIndex;
	}
}*/

int AST::GetConsecutiveTokenNumber(TokenType type)
{
	int tokenNum = 0;
	while (GetCurrentToken().type == type)
	{
		++tokenNum;
		++currentIndex;
	}
	return tokenNum;
}

int AST::GetConsecutiveTokenNumberWithoutUpdatingCurrentIndex(TokenType type, int init_offset)
{
	int tokenNum = 0;
	int offset = 0;
	while (tokens.at(currentIndex + init_offset + offset).type == type)
	{
		++tokenNum;
		++offset;
	}

	return tokenNum;
}


LValueType AST::GetTypeFromName(string&& name)
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