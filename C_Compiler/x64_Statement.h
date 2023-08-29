#pragma once
#include <string>
#include <vector>
#include "Variable.h"
#include <sstream>

using std::stringstream;
using std::string;
using std::vector;
using namespace VariableNamespace;

//enum RegisterSize
//{
//	BYTE,
//	WORD,
//	DWORD,
//	QWORD
//};

struct RegisterAsm
{
	REGISTER reg; //figure out how to treat special registers differently (i.e. rsp, rbp, rax, etc...)
	int size = REGISTER_SIZE;

	RegisterAsm();
	RegisterAsm(REGISTER reg);
};

enum OperandTypeAsm
{
	ASM_REG,
	ASM_INT_LITERAL,
	ASM_GLOBAL_MEMORY
};
struct OperandAsm
{
	OperandTypeAsm type = ASM_REG;
	RegisterAsm reg = RegisterAsm(_NONE);
	bool dereference = false;
	int baseOffset = 0;
	RegisterAsm regOffset;
	bool useRegOffset = false;
	int regOffsetMultiplier = 0;

	int literalIntValue = 0;

	static OperandAsm CreateRSPOffsetOperand(int offset);
	static OperandAsm CreateRBPOffsetOperand(int offset);
	static OperandAsm CreateOffsetOperand(memoryOffset offset);

	static OperandAsm CreateRegisterOperand(REGISTER reg);

	static OperandAsm CreateIntLiteralOperand(int value);

	string ToString() const;
};

enum StatementAsmType
{
	x64_NONE,
	//formatting of x64 script
	x64_DATA_SECTION,
	x64_CODE_SECTION,
	x64_FUNCTION_PROC,
	x64_FUNCTION_END,
	x64_CODE_END,
	x64_GLOBAL_VARIABLE,

	//actual x64 instructions
	x64_MOV,
	x64_MOVS,
	x64_ADD,
	x64_SUB,
	x64_IMUL,
	x64_IDIV, //TODO: figure out how to deal with register allocation with DIV
	x64_NEG, //TODO: figure out NEG for float
	x64_ADDS,
	x64_SUBS,
	x64_MULS,
	x64_DIVS,
	x64_JMP,
	x64_LABEL,
	x64_LEA,
	x64_CMP,
	x64_RET,
	x64_CALL,
	x64_SET,
	x64_FLOAT_TO_INT,
	x64_INT_TO_FLOAT,
	x64_PUSH,
	x64_POP,
	x64_CQO, //used before IDIV instruction (converts RAX to RDX:RAX)
	x64_CVTSI2SD, //long to double
	x64_CVTTSD2SI, //double to long
	x64_NOP
	//TODO: FLOAT COMPARE STATEMENT
};
struct StatementAsm
{
	StatementAsmType type;
	OperandAsm firstOperand;
	OperandAsm secondOperand;

	string name;
	FlagResults flags;

	vector<StatementAsm> preStatements;
	vector<StatementAsm> postStatements;


	string ToString() const;
	StatementAsm();
	StatementAsm(StatementAsmType type);
	StatementAsm(StatementAsmType type, string name);

};



/*
Compiling function to x64:

- if returning large struct, subtract size of struct from rsp (allocate space for return value) and store the address of this space in rax
- before calling function, push/copy all variables to the stack 
	- have dictionary that maps parameter/arg variable to the stack offset

- push rbp
- mov rbp, rsp

- subtract from rsp the total memory of the local variables defined/used in the function

- push all of the registers that will be used in this function to the stack

- compile body of function

- move return value to rax (if bigger than 8 bytes, don't do anything with rax because it already has the address of where the data of the struct is on the stack)

- pop all of the registers that were pushed to the stack

- add back to rsp the total memory of the local variables

- pop rbp

- call ret with value of the total size of the function params/args (i.e. use "ret 16" after calling a function that takes 4 ints)
*/



/*
Dealing with structs and arrays:
- when assigning specific variable (i.e. struct_var.member_var), get offset of the member_var and mov value into offset of start of struct
- in compiler, keep track of the start address (relative to rsp) of the struct variable
- when copying struct variables (i.e. struct var v2 = v1;), copy elements individually based on their type. Do this recursively for nested structs.
	- Have function that generates assign statements, if assigning for int/float, just return x64 statement; if it's a struct, call assign again for each of the member variables
*/

/*
- have all local variables be stored on the stack
- when allocating registers, keep track of which variables are stored in which registers at a given moment, so you don't have to keep writing and reading from stack memory
	every time you need to access/modify the contents of a variable.
*/