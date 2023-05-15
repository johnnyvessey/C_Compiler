#pragma once
#include <string>
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
		size_t periodCount = 0;
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
		return c == ' ' || c == '\t';
	}

}