#pragma once
#include "AST_Expression.h"

using namespace AST_Expression;

namespace VariableNamespace {
	struct Variable {
		string name;
		LValueType type;
		string structName;
		bool isPointer;
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