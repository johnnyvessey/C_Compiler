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
		VariableType(LValueType type, string structName, int pointerLevel);
	};

	inline bool IsNumericType(LValueType type)
	{
		return type == INT || type == FLOAT || type == BOOL;
	}

	struct Variable {
		string name;
		VariableType type;
		size_t arraySize = 0;
	};


	struct Struct_Variable {
		Variable v;
		size_t memoryOffset;
	};

	//needs to have access to scopeStack
	// 
	//inline size_t GetMemoryFromType(LValueType type, string structName = "")
	//{
	//	if (type == LValueType::INT || type == LValueType::FLOAT)
	//	{
	//		return 4;
	//	}
	//	else if (type == LValueType::STRUCT)
	//	{
	//		return 0; //TODO: FIGURE OUT HOW TO GET REFERENCE TO SPECIFIC STRUCT
	//	}
	//}
}