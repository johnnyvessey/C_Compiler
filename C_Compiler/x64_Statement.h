#pragma once
#include <string>
#include <vector>
using std::string;
using std::vector;


enum RegisterSize
{
	BYTE,
	WORD,
	DWORD,
	QWORD
};

struct x64_Register
{
	int registerIndex; //figure out how to treat special registers differently (i.e. rsp, rbp, rax, etc...)
	RegisterSize size;

};

struct x64_Statement
{

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