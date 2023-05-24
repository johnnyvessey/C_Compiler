#include "Lexer.h"

Token Lexer::ParseToken(const string&& s, size_t& lineNum, size_t& tokenNum)
{
	++tokenNum;
	//check if token
	if (tokenMap.find(s) != tokenMap.end())
	{
		return Token(tokenMap.at(s), s, lineNum, tokenNum);
	}
	if (std::all_of(s.begin(), s.end(), [](char c) {return Utils::isNumeric(c); }))
	{
		return Token(TokenType::INT_LITERAL, s, lineNum, tokenNum);
	}
	if (Utils::isAlpha(s[0]) && std::all_of(s.begin(), s.end(), [](char c) {return Utils::isAlpha(c) || Utils::isNumeric(c); }))
	{
		return Token(TokenType::NAME, s, lineNum, tokenNum);
	}
	if (Utils::isFloat(s))
	{
		return Token(TokenType::FLOAT_LITERAL, s, lineNum, tokenNum);
	}

	throw("Error Lexing string: " + s);

}

//KNOWN BUG: WILL PARSE WRONG WHEN THERE IS A STRING WITH SPACES/OTHER SPECIAL CHARACTERS
vector<Token> Lexer::SplitStringByToken(const string& input)
{
	const string doubleOps = "+-&|=<>*/%"; //TODO: move '/' here and figure out what to do about comments
	const string singleOps = "^(){}; \t\n,"; //[remove \n for now] //TODO: handle *=, -=, +=, /=, and %= 
	vector<Token> output;
	size_t start = 0;
	size_t lineNum = 1;
	size_t tokenNum = -1;

	for (size_t i = 0; i < input.size(); ++i)
	{
		char c = input.at(i);
		if (doubleOps.find(c) != string::npos)
		{
			if (start != i) {
				output.push_back(ParseToken(input.substr(start, i - start), lineNum, tokenNum));
			}
			if (i + 1 < input.size())
			{
				string doubleOpStr = input.substr(i, 2);
				if (tokenMap.find(doubleOpStr) != tokenMap.end())
				{
					output.push_back(ParseToken(std::move(doubleOpStr), lineNum, tokenNum));
					++i;
				}
				else {
					output.push_back(ParseToken(input.substr(i, 1), lineNum, tokenNum));
				}
			}
			else {
				output.push_back(ParseToken(input.substr(i, 1), lineNum, tokenNum));
			}

			start = i + 1;
		}
		else if (singleOps.find(c) != string::npos)
		{
			if (start != i) {
				output.push_back(ParseToken(input.substr(start, i - start), lineNum, tokenNum));
			}
			if (!Utils::isWhitespace(c))
			{
				output.push_back(ParseToken(input.substr(i, 1), lineNum, tokenNum));
			}

			if (c == '\n') {
				++lineNum;
			}
			start = i + 1;
		}
		else if (c == '.')
		{
			string prev = input.substr(start, i - start);
			if (!std::all_of(prev.begin(), prev.end(), [](char c) {return Utils::isNumeric(c); }))
			{
				output.push_back(ParseToken(std::move(prev), lineNum, tokenNum));
				output.push_back(ParseToken(input.substr(i, 1), lineNum, tokenNum));
				start = i + 1;
			}

		}

	}

	if (start != input.size() && !Utils::isWhitespace(input.at(input.size() - 1)))
	{
		output.push_back(ParseToken(input.substr(start, input.size() - start), lineNum, tokenNum));
	}

	output.push_back(Token(TokenType::END_OF_FILE, "END OF FILE", 0, 0));

	return output;
}

string Lexer::GetNameFromEnum(TokenType& type)
{
	for (const auto& pair : tokenMap)
	{
		if (pair.second == type)
		{
			return pair.first;
		}
	}

	return "name";
}

bool Lexer::IsBinOp(TokenType type)
{
	return type == TokenType::PLUS || type == TokenType::MINUS || type == TokenType::STAR || type == TokenType::SLASH || type == TokenType::PERCENT ||
		type == TokenType::DOUBLE_EQUAL || type == TokenType::NOT_EQUALS || type == TokenType::LESS_THAN || type == TokenType::LESS_THAN_EQUALS ||
		type == TokenType::GREATER_THAN || type == TokenType::GREATER_THAN_EQUALS;
}

bool Lexer::IsUnaryOp(TokenType type)
{
	return type == TokenType::PLUS_PLUS || type == TokenType::MINUS_MINUS;
}

bool Lexer::IsBinaryAssignmentOp(TokenType type)
{
	return type == PLUS_EQUAL || type == MINUS_EQUAL || type == STAR_EQUAL || type == SLASH_EQUAL || type == PERCENT_EQUAL;
}

bool Lexer::IsAssignmentOp(TokenType type)
{
	return Lexer::IsBinaryAssignmentOp(type) || type == SINGLE_EQUAL;
}

unordered_map<string, TokenType> Lexer::tokenMap = {
	{"if", TokenType::IF},
	{"else", TokenType::ELSE},
	{"int", TokenType::TYPE},
	{"float", TokenType::TYPE},
	{"char", TokenType::TYPE},
	{"(", TokenType::OPEN_PAR},
	{")", TokenType::CLOSE_PAR},
	{"{", TokenType::OPEN_BRACE},
	{"}", TokenType::CLOSE_BRACE},
	{";", TokenType::SEMICOLON},
	{"=", TokenType::SINGLE_EQUAL},
	{"==", TokenType::DOUBLE_EQUAL},
	{"+", TokenType::PLUS},
	{"-", TokenType::MINUS},
	{"++", TokenType::PLUS_PLUS},
	{"--", TokenType::MINUS_MINUS},
	{".", TokenType::PERIOD},
	{"struct", TokenType::STRUCT},
	{"\n", TokenType::NEW_LINE},
	{"*", TokenType::STAR},
	{"[", TokenType::OPEN_BRACKET},
	{"]", TokenType::CLOSE_BRACKET},
	{"for", TokenType::FOR},
	{"while", TokenType::WHILE},
	{"&", TokenType::ADDRESS_OF},
	{"&&", TokenType::AND}, //To make things easier for now, don't allow "bitwise and"
	{"||", TokenType::OR},
	{"/", TokenType::SLASH},
	{"%", TokenType::PERCENT},
	{",", TokenType::COMMA},
	{"->", TokenType::ARROW}, //TODO: Properly catch this in the lexer
	{"<", TokenType::LESS_THAN},
	{"<=", TokenType::LESS_THAN_EQUALS},
	{">", TokenType::GREATER_THAN},
	{">=", TokenType::GREATER_THAN_EQUALS},
	{"!=", TokenType::NOT_EQUALS},
	{"+=", TokenType::PLUS_EQUAL},
	{"-=", TokenType::MINUS_EQUAL},
	{"*=", TokenType::STAR_EQUAL},
	{"/=", TokenType::SLASH_EQUAL},
	{"%=", TokenType::PERCENT_EQUAL},
	{"void", TokenType::VOID},
	{"return", TokenType::RETURN}
};