#pragma once

#include <vector>
//#include "Parser.h"
#include "IR_Statement.h"
#include "Variable.h"
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

using std::vector;
using std::shared_ptr;
using std::make_shared;
using std::unordered_set;
using std::unordered_map;

using namespace VariableNamespace;

struct IR_Scope
{
	int scopeIndex = 1;
	vector<unordered_map<string, IR_Value>> variableMapping;
	vector<unordered_map<string, StructDefinition>> structMapping;
	unordered_map<string, IR_FunctionLabel> functionMapping;
	int currentLoopStartLabelIdx = 0;
	int currentLoopEndLabelIdx = 0;
	int functionEndLabel = 0;
	string currentFunction = "";
	IR_Value FindVariable(string name);
	StructDefinition FindStruct(string name);
	IR_FunctionLabel FindFunction(string name);

	vector<IR_Value> globalVariables;
	IR_Scope();
};

struct IR_State
{
	int varIndex = 1;
	IR_Scope scope;
	int labelIndex = 1;
	int currentFramePointerOffset = 0; //offset from stack pointer in current scope (for instance, if you have already stored 2 ints in the stack, the offset would be 8)
	//Scope scopeStack;
	const IR_Value functionReturnValueInt; //RAX register in x64
	const IR_Value functionReturnValueFloat; //XMM0 register
	const IR_Value functionReturnValueStructPointer; //pointer to struct return (first register passed into function) 
	const IR_Value functionVoidReturn;
	IR_Value flags; //flags set based on operations (especially used for boolean values)

	IR_State();
};

struct IR_Function_Group
{
	string functionName;
	vector<shared_ptr<IR_Statement>> IR_statements;

	IR_Function_Group(string functionName);
};

struct IR
{
	IR_State state;
	vector<IR_Function_Group> functions;
	vector<uint64_t> floatLiteralGlobals; //start at 1 rather than 0 for indexing names

	inline void add_statement(shared_ptr<IR_Statement> statement)
	{
		if (state.scope.currentFunction == "")
		{
			std::cout << "Error: statement not in function";
			throw 0;
		}

		functions.back().IR_statements.push_back(statement);
	}

	inline void add_floatLiteralGlobal(string s)
	{
		double d = std::stod(s);
		uint64_t intVal = *((uint64_t*)&d);
		floatLiteralGlobals.push_back(intVal);
	}

	//vector<shared_ptr<IR_Statement>> IR_statements;
	void EnterScope();
	void ExitScope();

	void EnterFunction();
	void ExitFunction();

	void DetermineRegisterStatusOfOperand(IR_Operand& op, unordered_map<int, int>& nonRegisterVariableMap);

	IR_VariableData ComputeIRVariableData();

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