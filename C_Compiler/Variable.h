#pragma once

#include <string>
using std::unique_ptr;
using std::string;

namespace VariableNamespace {

	enum LValueType {
		INT,
		FLOAT,
		STRUCT,
		BOOL
	};

	bool IsNumericType(LValueType type)
	{
		return type == INT || type == FLOAT || type == BOOL;
	}

	struct Value {
		string name;
		LValueType type;
		string structName;
		bool isReference;

	};
	struct Variable : Value {
		Variable() 
		{
			isReference = false;
		}
	};

	struct Pointer : Value
	{
		unique_ptr<Value> value;

		Pointer()
		{
			isReference = true;
		}
	};

	struct Struct_Variable {
		Variable v;
		size_t memoryOffset;
	};

	size_t GetMemoryFromType(LValueType type, string structName = "")
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