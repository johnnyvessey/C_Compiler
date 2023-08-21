#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "x64_State.h"

using std::vector;
using std::string;
using std::unordered_map;

#define POINTER_SIZE 8


enum IR_StatementType
{
	_IR_ASSIGN,
	_IR_VARIABLE_INIT,
	_IR_JUMP,
	_IR_LABEL,
	_IR_CONTINUOUS_MEMORY_INIT,
	_IR_FUNCTION_CALL,
	_IR_SCOPE_START,
	_IR_SCOPE_END,
	_IR_RETURN,
	_IR_FUNCTION_ARG_ASSIGN,
	_IR_VARIABLE_RELOAD,
	_IR_REGISTER_WRITE_TO_MEMORY,
	_IR_COMPARE,
	_IR_NOP,
	_IR_LOOP_START,
	_IR_LOOP_END,
	_IR_FUNCTION_LABEL,
	_IR_FUNCTION_START,
	_IR_FUNCTION_END
};
struct IR_Statement
{
	virtual string ToString() = 0;
	virtual IR_StatementType GetType() = 0;
	virtual void ConvertToX64(x64_State& state) = 0;
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


//struct IR_SructVariable : IR_Variable 
//{
//	
//};


//figure out how much struct info should be in the IR

//struct IR_Initialize : IR_Statement
//{
//	/*
//		For AST_Intialize, use this statement AS WELL AS IR_Assign. This statement will let the state know how much to subtract from stack pointer.
//	
//	*/
//	IR_VarType type;
//	IR_Variable var;
//
//};

struct IR_VariableInit : IR_Statement {
	IR_Value dest;

	IR_VariableInit();
	IR_VariableInit(IR_Value value);

	virtual string ToString() override;
	virtual IR_StatementType GetType() override;
	virtual void ConvertToX64(x64_State& state) override;
};

struct IR_Assign : IR_Statement
{
	IR_VarType type;
	IR_AssignType assignType; 
	
	IR_Operand dest;
	IR_Operand source;

	int byteSize;

	virtual string ToString() override;
	virtual IR_StatementType GetType() override;
	virtual void ConvertToX64(x64_State& state) override;

	IR_Assign();
	IR_Assign(IR_VarType type, IR_AssignType assignType, int byteSize, IR_Operand dest, IR_Operand source);
};

struct IR_ContinuousMemoryInit : IR_Statement
{
	int varIdx;
	int byteNum;
	bool isStruct;

	virtual string ToString() override;
	virtual IR_StatementType GetType() override;
	virtual void ConvertToX64(x64_State& state) override;

};

struct IR_Label : IR_Statement
{
	int label;

	virtual string ToString() override;
	virtual IR_StatementType GetType() override;
	virtual void ConvertToX64(x64_State& state) override;

	IR_Label(int label);

};

struct IR_ScopeStart : IR_Statement
{
	virtual string ToString() override;
	virtual IR_StatementType GetType() override;
	virtual void ConvertToX64(x64_State& state) override;
};

struct IR_ScopeEnd : IR_Statement
{
	virtual string ToString() override;
	virtual IR_StatementType GetType() override;
	virtual void ConvertToX64(x64_State& state) override;
};

struct IR_Jump : IR_Statement
{
	int labelIdx;
	FlagResults condition = IR_ALWAYS;

	virtual string ToString() override;
	virtual IR_StatementType GetType() override;
	virtual void ConvertToX64(x64_State& state) override;

	IR_Jump(int labelIdx, FlagResults condition = IR_ALWAYS);
};

struct IR_Return : IR_Statement
{
	virtual string ToString() override;
	virtual IR_StatementType GetType() override;
	virtual void ConvertToX64(x64_State& state) override;
};

//struct IR_ALUBinOp : IR_Statement
//{
//	IR_Variable result;
//	IR_Variable val1;
//	IR_Value val2;
//
//	IR_VarType resultType; //is int or float (or long/double); this will affect what instruction is used
//	ALUBinOpType binOpType;
//
//	virtual string ToString() override;
//};

struct IR_FunctionCall : IR_Statement
{
	string funcName;
	//vector<IR_Value> args;

	virtual string ToString() override;
	virtual IR_StatementType GetType() override;
	virtual void ConvertToX64(x64_State& state) override;

	IR_FunctionCall(string funcName);
};

enum IR_FunctionArgType
{
	IR_INT_ARG,
	IR_FLOAT_ARG,
	IR_STACK_ARG
};
struct IR_FunctionArgAssign : IR_Statement
{
	int argIdx;
	IR_Operand value;
	IR_FunctionArgType argType;
	int byteSize;
	int stackArgOffset;

	IR_FunctionArgAssign();
	IR_FunctionArgAssign(int argIdx, IR_Operand value, IR_FunctionArgType argType, int byteSize, int stackArgOffset);
	virtual void ConvertToX64(x64_State& state) override;

	virtual string ToString() override;
	virtual IR_StatementType GetType() override;

};

/*
	This is a statement that is added anytime after value in a pointer is modified. This is because all bets are off
	on whether a variable was affected by an aliasing pointer. Thus, each register that is active needs to reload their value
	from the stack. (Const variables do not need to be reloaded)
	
	MAYBE: This does not need to happen for temp variables that will only reside in registers and not the stack. (We might still
	need to reload if there is register spilling)

*/
//struct IR_VariableReload : IR_Statement
//{
//	virtual string ToString() override;
//	virtual IR_StatementType GetType() override;
//	virtual void ConvertToX64(x64_State& state) override;
//};
//
////called any time pointer value is read
//struct IR_RegisterWriteToMemory : IR_Statement
//{
//	virtual string ToString() override;
//	virtual IR_StatementType GetType() override;
//	virtual void ConvertToX64(x64_State& state) override;
//};

struct IR_Compare : IR_Statement
{
	IR_Operand op1;
	IR_Operand op2;

	virtual string ToString() override;
	virtual IR_StatementType GetType() override;
	virtual void ConvertToX64(x64_State& state) override;

	IR_Compare();
	IR_Compare(IR_Operand op1, IR_Operand op2);
};

struct IR_NOP : IR_Statement
{
	virtual string ToString() override;
	virtual IR_StatementType GetType() override;
	virtual void ConvertToX64(x64_State& state) override;
};

struct IR_LoopStart : IR_Statement
{
	virtual string ToString() override;
	virtual IR_StatementType GetType() override;
	virtual void ConvertToX64(x64_State& state) override;
};

struct IR_LoopEnd : IR_Statement
{
	virtual string ToString() override;
	virtual IR_StatementType GetType() override;
	virtual void ConvertToX64(x64_State& state) override;
};

struct IR_FunctionLabel : IR_Statement
{
	string functionName;
	vector<IR_Value> args;
	IR_Value returnValue;
	int returnValueByteSize;

	virtual string ToString() override;
	virtual IR_StatementType GetType() override;
	virtual void ConvertToX64(x64_State& state) override;

	IR_FunctionLabel(string functionName);
	IR_FunctionLabel();
};

struct IR_FunctionStart : IR_Statement
{
	virtual string ToString() override;
	virtual IR_StatementType GetType() override;
	virtual void ConvertToX64(x64_State& state) override;
};

struct IR_FunctionEnd : IR_Statement
{
	virtual string ToString() override;
	virtual IR_StatementType GetType() override;
	virtual void ConvertToX64(x64_State& state) override;
};

namespace IR_Utils
{
	static IR_Value ParseLiteral(string value, IR_VarType type)
	{
		IR_Value ir_val;
		ir_val.literalValue = value;
		ir_val.valueType = IR_LITERAL;

		return ir_val;
	}

}
