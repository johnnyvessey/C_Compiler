#pragma once

#include <unordered_map>
#include <string>
#include <algorithm>
#include <vector>

#include "Utils.h"

using std::unordered_map;
using std::string;
using std::vector;



namespace Lexer {

	enum TokenType {
		OPEN_BRACE,
		CLOSE_BRACE,
		OPEN_PAR,
		CLOSE_PAR,
		TYPE,
		SEMICOLON,
		RETURN,
		ELSE,
		IF,
		INT_LITERAL,
		FLOAT_LITERAL,
		STRING_LITERAL,
		CHAR_LITERAL,
		NAME,
		SINGLE_EQUAL,
		DOUBLE_EQUAL,
		PLUS,
		MINUS,
		PLUS_PLUS,
		MINUS_MINUS,
		PERIOD,
		STRUCT,
		NEW_LINE,
		STAR,
		SLASH,
		PERCENT,
		OPEN_BRACKET,
		CLOSE_BRACKET,
		END_OF_FILE,
		FOR,
		WHILE,
		ADDRESS_OF,
		AND,
		OR,
		COMMA,
		ARROW,
		LESS_THAN,
		LESS_THAN_EQUALS,
		GREATER_THAN,
		GREATER_THAN_EQUALS,
		NOT_EQUALS
	};

	struct Token {
		TokenType type;
		string value;
		size_t lineNumber;
		size_t tokenNumber;
		Token(TokenType type, string value, size_t lineNumber, size_t tokenNumber) : type(type), value(value), lineNumber(lineNumber), tokenNumber(tokenNumber) {}
		Token(TokenType type): type(type) {}
	};

	const static unordered_map<string, TokenType> tokenMap = {
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
		{"!=", TokenType::NOT_EQUALS}
	};


	bool IsBinOp(TokenType type);
	bool IsUnaryOp(TokenType type);
	Token ParseToken(const string&& s, size_t& lineNumber, size_t& tokenNum);
	vector<Token> SplitStringByToken(const string& input);
	string GetNameFromEnum(TokenType& type);


	Token ParseToken(const string&& s, size_t& lineNum, size_t& tokenNum)
	{
		++tokenNum;
		//check if token
		if (Lexer::tokenMap.find(s) != Lexer::tokenMap.end())
		{
			return Token(Lexer::tokenMap.at(s), s, lineNum, tokenNum);
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
	vector<Token> SplitStringByToken(const string& input)
	{
		const string doubleOps = "+-&|=<>"; //TODO: move '/' here and figure out what to do about comments
		const string singleOps = "*/%^(){}; \t\n"; //[remove \n for now] //TODO: handle *=, -=, +=, /=, and %= 
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

	string GetNameFromEnum(TokenType& type)
	{
		for (const auto& pair : Lexer::tokenMap)
		{
			if (pair.second == type)
			{
				return pair.first;
			}
		}

		return "name";
	}

	 bool IsBinOp(TokenType type)
	{
		return type == TokenType::PLUS || type == TokenType::MINUS || type == TokenType::STAR || type == TokenType::SLASH || type == TokenType::PERCENT || 
			type == TokenType::DOUBLE_EQUAL || type == TokenType::NOT_EQUALS || type == TokenType::LESS_THAN || type == TokenType::LESS_THAN_EQUALS || 
			type == TokenType::GREATER_THAN || type == TokenType::GREATER_THAN_EQUALS;
	}

	 bool IsUnaryOp(TokenType type)
	{
		return type == TokenType::PLUS_PLUS || type == TokenType::MINUS_MINUS;
	}

};