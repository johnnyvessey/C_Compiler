#pragma once

#include <string>
#include <vector>

using std::vector;
using std::string;

struct IR_Statement
{
	virtual string ToString() = 0;
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
	//string name;
	size_t varIndex;
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

	virtual string ToString() override;

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


struct IR_ALUBinOp : IR_Statement
{
	IR_Variable result;
	IR_Variable val1;
	IR_Value val2;

	IR_VarType resultType; //is int or float (or long/double); this will affect what instruction is used
	ALUBinOpType binOpType;

	virtual string ToString() override;
};

struct IR_FunctionCall : IR_Statement
{
	string funcName;
	vector<IR_Value> args;

	virtual string ToString() override;

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