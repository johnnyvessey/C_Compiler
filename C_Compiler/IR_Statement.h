#pragma once

#include <string>
#include <vector>
#include <unordered_map>

using std::vector;
using std::string;
using std::unordered_map;

struct IR_Statement
{
	virtual string ToString() = 0;
};

enum IR_AssignType
{
	COPY,
	IR_ADD,
	IR_SUBTRACT,
	IR_MULTIPLY,
	IR_DIVIDE, //divide in x64 is tricky because it uses eax, edx registers (look into this more)
	IR_LEFT_SHIFT,
	IR_RIGHT_SHIFT,
	IR_FUSED_MULTIPLY_ADD
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

struct IR_Value
{
	IR_VarType type;
	IR_ValueType valueType;

	int byteSize;
	size_t varIndex;
	bool isTempValue; //temp value in middle of expression (only needs to be in registers, won't be stored on the stack)
	string literalValue;
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


struct IR_Assign : IR_Statement
{
	IR_VarType type;
	IR_AssignType assignType; 
	
	IR_Value dest;
	IR_Value source;

	virtual string ToString() override;
	IR_Assign();
	IR_Assign(IR_VarType type, IR_AssignType assignType, IR_Value dest, IR_Value source);
};

struct IR_StructInit : IR_Statement
{
	size_t structVarIdx;
	int byteNum;
};

struct IR_Label : IR_Statement
{
	string label;

	virtual string ToString() override;

};

struct IR_ScopeStart : IR_Statement
{


	virtual string ToString() override;

};

struct IR_ScopeEnd : IR_Statement
{



	virtual string ToString() override;
};

struct IR_Branch : IR_Statement
{
	IR_Label dest;

	virtual string ToString() override;

};

struct IR_Return : IR_Statement
{
	virtual string ToString() override;

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

};

struct IR_FunctionArgAssign : IR_Statement
{
	size_t argIdx;
	IR_Value value;
};

struct IR_TypeCast : IR_Statement
{

	virtual string ToString() override;
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

};

//called any time pointer value is read
struct IR_RegisterWriteToMemory : IR_Statement
{
	virtual string ToString() override;

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