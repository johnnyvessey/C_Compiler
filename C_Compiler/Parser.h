#pragma once

#include <unordered_map>
#include <string>
#include <algorithm>
#include <vector>

#include "Utils.h"

using std::unordered_map;
using std::string;
using std::vector;



namespace Parser {

	enum TokenType {
		OPEN_BRACE,
		CLOSE_BRACE,
		OPEN_PAR,
		CLOSE_PAR,
		INT,
		FLOAT,
		SEMICOLON,
		RETURN,
		ELSE,
		IF,
		INT_LITERAL,
		FLOAT_LITERAL,
		STRING_LITERAL,
		NAME,
		SINGLE_EQUAL,
		DOUBLE_EQUAL,
		PLUS,
		MINUS,
		PLUS_PLUS,
		MINUS_MINUS,
		PERIOD,
		STRUCT
	};

	struct Token {
		TokenType type;
		string value;

		Token(TokenType type, string value) : type(type), value(value) {}
	};

	const static unordered_map<string, TokenType> tokenMap = {
		{"if", TokenType::IF},
		{"int", TokenType::INT},
		{"float", TokenType::FLOAT},
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
		{"struct", TokenType::STRUCT}
	};

	static Token ParseToken(const string&& s);
	static vector<Token> SplitStringByToken(const string& input);
	static string GetNameFromEnum(TokenType& type);


	static Token ParseToken(const string&& s)
	{
		//check if token
		if (Parser::tokenMap.find(s) != Parser::tokenMap.end())
		{
			return Token(Parser::tokenMap.at(s), "");
		}
		if (std::all_of(s.begin(), s.end(), [](char c) {return Utils::isNumeric(c); }))
		{
			return Token(TokenType::INT_LITERAL, s);
		}
		if (Utils::isAlpha(s[0]) && std::all_of(s.begin(), s.end(), [](char c) {return Utils::isAlpha(c) || Utils::isNumeric(c); }))
		{
			return Token(TokenType::NAME, s);
		}

		if (Utils::isFloat(s))
		{
			return Token(TokenType::FLOAT_LITERAL, s);
		}

		throw("Error Parsing string: " + s);

	}

	//KNOWN BUG: WILL PARSE WRONG WHEN THERE IS A STRING WITH SPACES/OTHER SPECIAL CHARACTERS
	static vector<Token> SplitStringByToken(const string& input)
	{
		//const string bracePar = "(){}; ";
		const string doubleOps = "+-&|=";
		const string singleOps = "*/%^(){}; "; //figure out what to do about period (struct accessing vs float literal)
		vector<Token> output;
		size_t start = 0;
		for (size_t i = 0; i < input.size(); ++i)
		{
			char c = input.at(i);
			if (doubleOps.find(c) != string::npos)
			{
				if (start != i) {
					output.push_back(ParseToken(input.substr(start, i - start)));
				}
				if (i + 1 < input.size() && input.at(i + 1) == c)
				{
					output.push_back(ParseToken(input.substr(i, 2)));
					++i;
				}
				else {
					output.push_back(ParseToken(input.substr(i, 1)));
				}

				start = i + 1;

			}
			else if (singleOps.find(c) != string::npos)
			{
				if (start != i) {
					output.push_back(ParseToken(input.substr(start, i - start)));
				}
				if (c != ' ')
				{
					output.push_back(ParseToken(input.substr(i, 1)));
				}
				start = i + 1;

			}
			else if (c == '.')
			{
				string prev = input.substr(start, i - start);
				if (!std::all_of(prev.begin(), prev.end(), [](char c) {return Utils::isNumeric(c); }))
				{
					output.push_back(ParseToken(std::move(prev)));
					output.push_back(ParseToken(input.substr(i, 1)));
					start = i + 1;
				}

			}

		}

		if (start != input.size())
		{
			output.push_back(ParseToken(input.substr(start, input.size() - start)));
		}

		return output;
	}

	static string GetNameFromEnum(TokenType& type)
	{
		for (const auto& pair : Parser::tokenMap)
		{
			if (pair.second == type)
			{
				return pair.first;
			}
		}

		return "name";
	}



};