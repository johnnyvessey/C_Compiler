#pragma once

#include <unordered_map>
#include <string>
#include <algorithm>
#include <vector>

#include "Utils.h"

using std::unordered_map;
using std::string;
using std::vector;


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
	NOT_EQUALS,
	PLUS_EQUAL,
	MINUS_EQUAL,
	STAR_EQUAL,
	SLASH_EQUAL,
	PERCENT_EQUAL,
	VOID
};

	struct Token {
		TokenType type;
		string value;
		size_t lineNumber;
		size_t tokenNumber;
		Token(TokenType type, string value, size_t lineNumber, size_t tokenNumber) : type(type), value(value), lineNumber(lineNumber), tokenNumber(tokenNumber) {}
		Token(TokenType type): type(type) {}
	};



struct Lexer {
	static bool IsBinOp(TokenType type);
	static bool IsUnaryAssignmentOp(TokenType type);
	static Token ParseToken(const string&& s, size_t& lineNumber, size_t& tokenNum);
	static vector<Token> SplitStringByToken(const string& input);
	static string GetNameFromEnum(TokenType& type);
	static bool IsBinaryAssignmentOp(TokenType type);
	static bool IsAssignmentOp(TokenType type);

	static bool IsNonBinaryExpressionTerminalToken(TokenType type);
	static unordered_map<string, TokenType> tokenMap;
};