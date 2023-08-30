#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using std::unique_ptr;
using std::string;
using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::pair;

#define POINTER_SIZE 8
#define REGISTER_SIZE 8

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
		_NONE = -1,
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

	struct RegisterString
	{
		const static vector<string> registerStringMapping;

	};


	enum IR_AssignType
	{
		IR_COPY,
		IR_TYPE_CAST,
		IR_NEGATIVE,
		IR_ADD,
		IR_SUBTRACT,
		IR_MULTIPLY,
		IR_DIVIDE, //divide in x64 is tricky because it uses eax, edx registers (look into this more)
		IR_LEFT_SHIFT,
		IR_RIGHT_SHIFT,
		IR_FUSED_MULTIPLY_ADD,
		IR_FLAG_CONVERT,
		IR_STRUCT_COPY,
		IR_LEA
	};


	enum IR_VarType
	{
		IR_INT,
		IR_FLOAT,
		IR_STRUCT
	};


	enum IR_ValueType
	{
		IR_LITERAL,
		IR_VARIABLE
	};


	enum IR_SpecialVars
	{
		IR_NONE,
		IR_FLAGS,
		IR_RETURN_INT,
		IR_RETURN_FLOAT,
		IR_RETURN_STACK
	};
	struct IR_Value
	{
		IR_VarType type;
		IR_ValueType valueType;

		int byteSize;
		int varIndex;
		bool isTempValue; //temp value in middle of expression (only needs to be in registers, won't be stored on the stack)

		string literalValue;
		IR_SpecialVars specialVars = IR_NONE;
		FlagResults flag = IR_NO_FLAGS;

		int pointerLevel = 0;
		IR_VarType baseType; //this is for pointers to know what the base type it is

		IR_Value();
		IR_Value(IR_VarType type, IR_ValueType valueType, int byteSize, int varIndex, bool isTempValue = true, string literalValue = "", IR_SpecialVars specialVars = IR_NONE);
		IR_Value(IR_VarType type, IR_ValueType valueType, int byteSize, int varIndex, bool isTempValue, string literalValue, IR_SpecialVars specialVars, int pointerLevel, IR_VarType baseType);
	};

	struct IR_Operand
	{
		IR_Value value;
		bool dereference = false;

		bool useMemoryAddress = false;

		int baseOffset = 0;

		int memoryOffsetMultiplier = 0; //NOTE: this can only be 1,2,4,8
		IR_Value memoryOffset;

		int globalFloatValue = 0;
		IR_VarType GetVarType();
		int GetByteSize();
		int GetPointerLevel();

		IR_Operand();
		IR_Operand(IR_Value value);
	};

	struct IR_FunctionDef
	{
		vector<IR_Value> args;
		string functionName;
		IR_Value returnVar;
	};

	struct IR_VariableData
	{
		unordered_map<string, unordered_map<int, int>> nonRegisterVariables;
		unordered_map<string, unordered_map<int, vector<int>>> variableLineMapping;
		unordered_map<string, unordered_map<int, int>> normalIndexToDoubledIndexMapping;

		unordered_map<int, vector<int>>* currentLineMapping;
		unordered_map<int, int>* currentNormalIndexToDoubledIndexMapping;

		unordered_map<string, IR_FunctionDef> functionDefinitions;

		vector<vector<int>> irScopeStack;

	};


	struct RegisterVariableGroup
	{
		//REGISTER reg;
		int variableIndex = 0;
		bool isModified = true;

		//TODO: figure out if register size is necessary (i.e. RAX, EAX, AL...)
		RegisterVariableGroup();
		RegisterVariableGroup(int varIndex, bool isModified);
	};

	struct RegisterMapping
	{
		vector<RegisterVariableGroup> regMapping;
		RegisterMapping();
		void SetRegister(int reg, int variable);
		bool FindRegisterOfVariable(int variable, int& reg);
	};

	struct JumpRegisterMapping
	{
		RegisterMapping jumpRegMapping;
		int jumpStatementIndex;

		JumpRegisterMapping(RegisterMapping regMapping, int jumpStatementIndex);
	};

	struct LabelRegisterMaps
	{
		RegisterMapping initialMapping;
		vector<JumpRegisterMapping> jumpMappings;
	};

	struct memoryOffset
	{
		int offset;
		bool isRsp = true;

		memoryOffset(int offset, bool isRsp = true);
		memoryOffset();
	};

	struct MemoryVariableMapping
	{
		//TODO: clear this at start of every function
		unordered_map<int, int> memoryOffsetMapping; //maps varIndex to offset from RSP pointer (positive for arguments, negative for local variables)
		unordered_map<int, memoryOffset> memoryOffsetMappingSpilledRegisters; //maps varIndex to offset from RSP pointer (positive for arguments, negative for local variables)

	};

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