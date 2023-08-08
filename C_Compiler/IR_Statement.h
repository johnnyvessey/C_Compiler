#pragma once

#include <string>
#include <vector>
#include <unordered_map>

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
	_IR_STRUCT_INIT,
	_IR_FUNCTION_CALL,
	_IR_SCOPE_START,
	_IR_SCOPE_END,
	_IR_RETURN,
	_IR_FUNCTION_ARG_ASSIGN,
	_IR_VARIABLE_RELOAD,
	_IR_REGISTER_WRITE_TO_MEMORY,
	_IR_COMPARE,
	_IR_NOP
};
struct IR_Statement
{
	virtual string ToString() = 0;
	virtual IR_StatementType GetType() = 0;
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
	IR_STRUCT_COPY
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

enum IR_FlagResults
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
};

enum IR_SpecialVars
{
	IR_NONE,
	IR_FLAGS,
	IR_RETURN
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
	IR_FlagResults flag = IR_NO_FLAGS;

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

	IR_Assign();
	IR_Assign(IR_VarType type, IR_AssignType assignType, int byteSize, IR_Operand dest, IR_Operand source);
};

struct IR_ContinuousMemoryInit : IR_Statement
{
	int varIdx;
	int byteNum;

	virtual string ToString() override;
	virtual IR_StatementType GetType() override;

};

struct IR_Label : IR_Statement
{
	int label;

	virtual string ToString() override;
	virtual IR_StatementType GetType() override;


	IR_Label(int label);

};

struct IR_ScopeStart : IR_Statement
{


	virtual string ToString() override;
	virtual IR_StatementType GetType() override;


};

struct IR_ScopeEnd : IR_Statement
{



	virtual string ToString() override;
	virtual IR_StatementType GetType() override;

};

struct IR_Jump : IR_Statement
{
	int labelIdx;
	IR_FlagResults condition = IR_ALWAYS;

	virtual string ToString() override;
	virtual IR_StatementType GetType() override;

	IR_Jump(int labelIdx, IR_FlagResults condition = IR_ALWAYS);

};

struct IR_Return : IR_Statement
{
	virtual string ToString() override;
	virtual IR_StatementType GetType() override;


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
	vector<IR_Value> args;

	virtual string ToString() override;
	virtual IR_StatementType GetType() override;

};

struct IR_FunctionArgAssign : IR_Statement
{
	int argIdx;
	IR_Operand value;

	IR_FunctionArgAssign();
	IR_FunctionArgAssign(int argIdx, IR_Operand value);

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
struct IR_VariableReload : IR_Statement
{


	virtual string ToString() override;
	virtual IR_StatementType GetType() override;

};

//called any time pointer value is read
struct IR_RegisterWriteToMemory : IR_Statement
{
	virtual string ToString() override;
	virtual IR_StatementType GetType() override;


};

struct IR_Compare : IR_Statement
{
	IR_Operand op1;
	IR_Operand op2;

	virtual string ToString() override;
	virtual IR_StatementType GetType() override;


	IR_Compare();
	IR_Compare(IR_Operand op1, IR_Operand op2);
};

struct IR_NOP : IR_Statement
{
	virtual string ToString() override;
	virtual IR_StatementType GetType() override;
};

//struct IR_Assign_From_Flags : IR_Statement
//{
//	IR_Value dest;
//	IR_FlagResults flag;
//	virtual string ToString() override;
//
//	IR_Assign_From_Flags(IR_Value dest, IR_FlagResults flag);
//};

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
