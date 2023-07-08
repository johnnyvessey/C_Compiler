#pragma once

#include <string>

using std::string;

struct IR_Statement
{

};

enum ALUBinOpType
{
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
	INT,
	FLOAT,
	LONG
};


struct IR_Value
{
	IR_VarType type;
	bool isTempValue; //temp value in middle of expression (only needs to be in registers, won't be stored on the stack)
};

struct IR_Variable : IR_Value
{
	string name;
};

struct IR_Int_Literal : IR_Value
{
	int value;
};

struct IR_Float_Literal : IR_Value
{
	float value; //need to have this be a constant in the data section and then load it into stack/float register when you need to use it
};


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
	IR_Variable var;
	IR_Value result;
};


struct IR_Label : IR_Statement
{
	string label;
};

struct IR_Scope_Start : IR_Statement
{

};

struct IR_Scope_End : IR_Statement
{

};

struct IR_Branch : IR_Statement
{
	IR_Label dest;
};


struct IR_ALUBinOp : IR_Statement
{
	IR_Variable result;
	IR_Variable val1;
	IR_Value val2;

	IR_VarType resultType; //is int or float (or long/double); this will affect what instruction is used
	ALUBinOpType binOpType;
};