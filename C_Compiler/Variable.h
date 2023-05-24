#pragma once

#include <string>
#include <memory>

using std::unique_ptr;
using std::string;

namespace VariableNamespace {

	enum LValueType {
		VOID,
		INT,
		FLOAT,
		STRUCT,
		BOOL	
	};

	struct VariableType
	{
		LValueType lValueType;
		string structName;
		int pointerLevel;

		bool operator==(const VariableType& rhs);
		VariableType();
	};

	inline bool IsNumericType(LValueType type)
	{
		return type == INT || type == FLOAT || type == BOOL;
	}

	struct Variable {
		string name;
		//LValueType type;
		//string structName;
		//int pointerLevel; //0 for value types, 1 for int* x, 2 for int** x, etc...
		VariableType type;
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