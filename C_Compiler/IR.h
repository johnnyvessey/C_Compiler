#pragma once

#include <vector>
//#include "Parser.h"
#include "IR_Statement.h"
#include "Variable.h"

using std::vector;
using std::shared_ptr;
using std::make_shared;
using namespace VariableNamespace;
/*
TODO:
figure out how to represent struct member variables in IR
	- for example:
	struct var v;
	v.x = 1; //how do we represent v.x in IR? Is it its own variable? Is is just a pointer to the struct variable with an offset?

	
*/

//struct IRStructVariable 
//{
//	IR_VarType type;
//	bool isArray;
//	string structName;
//	int arrayLength = 0;
//};
//
//struct IRStructDefinition
//{
//	string name;
//
//	vector<IRStructVariable> variables;
//
//	int memorySize;
//};
struct IR_Scope
{
	int scopeIndex = 1;
	vector<unordered_map<string, IR_Value>> variableMapping;
	vector<unordered_map<string, StructDefinition>> structMapping;
	unordered_map<string, FunctionDefinition> functionMapping;
	int currentLoopStartLabelIdx = 0;
	int currentLoopEndLabelIdx = 0;

	IR_Value FindVariable(string name);
	StructDefinition FindStruct(string name);
	FunctionDefinition FindFunction(string name);


	IR_Scope();
};

struct IR_State
{
	int varIndex = 1;
	IR_Scope scope;
	int labelIndex = 1;
	int currentStackPointerOffset = 0; //offset from stack pointer in current scope (for instance, if you have already stored 2 ints in the stack, the offset would be 8)
	//Scope scopeStack;
	IR_Value functionReturnValue; //RAX register in x64
	IR_Value flags; //flags set based on operations (especially used for boolean values)

	IR_State();
};

struct IR
{
	IR_State state;
	vector<shared_ptr<IR_Statement>> IR_statements;
	void EnterScope();
	void ExitScope();

	//vector<IR_Statement> Function_Statements;
	//vector<IR_Statement> Data_Statements;

	//probably don't need struct defs for now; just the fact that a initialization is a struct
	//unordered_map<string, IRStructDefinition> IR_structs; //definition of IR structs (like LLVM structs... very simple; include number (based on scope) to distinguish between
	//structs with the same name

	IR();

};



/*
When converting IR to x64, have dictionaries that maps variable to register and place in memory.
When you get to memoryReload command, clear the register dict
When you get to a writeToMemory command, insert commands to write all live registers to their place in memory.
	If a register has not been written to memory, decrement stack pointer (do this in bulk at the beginning of a scope)

All IR variables could be put in registers.
*/