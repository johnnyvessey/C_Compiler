#pragma once

#include <vector>
//#include "Parser.h"
#include "IR_Statement.h"
//#include "Scope.h"

using std::vector;

struct IR_State
{
	size_t tmpVarIndex = 1;
	size_t labelIndex = 0;
	int currentStackPointerOffset = 0; //offset from stack pointer in current scope (for instance, if you have already stored 2 ints in the stack, the offset would be 8)
	//Scope scopeStack;
	size_t scopeIndex = 0; //every time scope index changes (go forward or back in scope), add one to it. When you declare a variable append _{scopeIndex} to it
};

struct IR
{
	IR_State state;
	vector<IR_Statement> IR_statements;
	vector<IR_Statement> Function_Statements;
	vector<IR_Statement> Data_Statements;
};