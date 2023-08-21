#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

using std::unique_ptr;
using std::string;
using std::vector;
using std::unordered_map;

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
		int arraySize = 0;
		bool isConst = false;
		//int IR_varNum;
	};

	struct FunctionDefinition {
		string name;
		vector<Variable> arguments;
		VariableType returnType;
	};

	struct Struct_Variable {
		Variable v;
		int memoryOffset;
	};

	struct StructDefinition
	{
		string name;
		unordered_map<string, Struct_Variable> variableMapping;
		vector<Struct_Variable> variableVector;
		int memorySize;
	};

	inline int GetMemorySizeForIR(VariableType type, StructDefinition* structDef = nullptr)
	{
		if (type.pointerLevel > 0)
		{
			return 8;
		}
		else if (type.lValueType == LValueType::INT || type.lValueType == LValueType::FLOAT || type.lValueType == LValueType::BOOL)
		{
			return 4;
		}
		else if (type.lValueType == LValueType::STRUCT)
		{
			return structDef->memorySize;
		}
		else {
			//none
			throw 0;
		}


	}

	enum FlagResults
	{
		IR_NO_FLAGS = 0,
		IR_ALWAYS = 1,
		IR_NEVER = -1,
		IR_GREATER = 2,
		IR_LESS_EQUALS = -2,
		IR_GREATER_EQUALS = 3,
		IR_LESS = -3,
		IR_EQUALS = 4,
		IR_NOT_EQUALS = -4,
		IR_FLOAT_GREATER = 5,
		IR_FLOAT_LESS_EQUALS = -5,
		IR_FLOAT_LESS = -6,
		IR_FLOAT_GREATER_EQUALS = 6
	};

#define NUM_REGISTERS 32
	enum REGISTER
	{
		RAX,
		RBX,
		RCX,
		RDX,
		RSI,
		RDI,
		RBP,
		RSP,
		R8,
		R9,
		R10,
		R11,
		R12,
		R13,
		R14,
		R15,
		XMM0,
		XMM1,
		XMM2,
		XMM3,
		XMM4,
		XMM5,
		XMM6,
		XMM7,
		XMM8,
		XMM9,
		XMM10,
		XMM11,
		XMM12,
		XMM13,
		XMM14,
		XMM15
	};

	const static vector<string> REGISTER_STRING;


	//inline int GetMemorySizeForIR(VariableType type, IR& irState, string structName = "")
	//{
	//	if (type.pointerLevel > 0)
	//	{
	//		return 8;
	//	}
	//	else if (type.lValueType == LValueType::INT || type.lValueType == LValueType::FLOAT || type.lValueType == LValueType::BOOL)
	//	{
	//		return 4;
	//	}
	//	else if (type.lValueType == LValueType::STRUCT)
	//	{
	//		StructDefinition structDef = irState.state.scope.FindStruct(structName);
	//		return structDef.memorySize;
	//	}
	//	else {
	//		//none
	//		throw 0;
	//	}


	//}
	//needs to have access to scopeStack
	// 
	//inline int GetMemoryFromType(LValueType type, string structName = "")
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