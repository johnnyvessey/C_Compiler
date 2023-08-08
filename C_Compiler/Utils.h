#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using std::string;


namespace Utils {
	inline bool isNumeric(const char& c)
	{
		return c >= '0' && c <= '9';
	}

	inline bool isAlpha(const char& c)
	{
		return c >= 'A' && c <= 'z';
	}

	inline bool isFloat(const string& s)
	{
		int periodCount = 0;
		bool isFloat = true;
		for (const char& c : s)
		{
			if (c == '.') ++periodCount;
			else if (c >= '0' && c <= '9') continue;
			else {
				return false;
			}
		}

		return periodCount == 1;
	}

	inline bool isWhitespace(const char& c)
	{
		return c == ' ' || c == '\t' || c == '\n';
	}

	inline string ReadFile(const string&& fileName)
	{
		std::ifstream test_code(fileName);
		std::stringstream buffer;
		buffer << test_code.rdbuf();

		return buffer.str();
	}

	inline void throwError(string message)
	{
		std::cout << "ERROR: " << message;
		throw 0;
	}

}