#pragma once

#include <string>
using std::unique_ptr;
using std::string;

namespace VariableNamespace {

	enum LValueType {
		INT,
		FLOAT,
		STRUCT,
		BOOL,
		VOID
	};

	inline bool IsNumericType(LValueType type)
	{
		return type == INT || type == FLOAT || type == BOOL;
	}

	struct Variable {
		string name;
		LValueType type;
		string structName;
		int pointerLevel; //0 for value types, 1 for int* x, 2 for int** x, etc...
	};

	struct Struct_Variable {
		Variable v;
		size_t memoryOffset;
	};

	inline size_t GetMemoryFromType(LValueType type, string structName = "")
	{
		if (type == LValueType::INT || type == LValueType::FLOAT)
		{
			return 4;
		}
		else if (type == LValueType::STRUCT)
		{
			return 0; //TODO: FIGURE OUT HOW TO GET REFERENCE TO SPECIFIC STRUCT
		}
	}
}