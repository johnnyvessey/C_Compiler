#pragma once

#include "AST_Statement.h"

using namespace AST_Statement;

/*
SCOPE of variables:
- vector functioning as stack
- push new scope object to stack when entering, pop when leaving scope (don't care about variables once we're out of scope)
- iterate through vector backwards to find the variable/function/struct name
*/
struct ScopeLevel {
	unordered_map<string, Variable> variables;
	unordered_map<string, AST_Struct_Definition> structs;
};

struct Scope {
	vector<ScopeLevel> scope;
	unordered_map<string, Function> functionScope; //functions cannot be declared within functions / inner scope

	Scope();
	bool TryFindVariable(const string& name, Variable& var);
	bool TryFindFunction(const string& name, Function& func);
	bool FunctionNameExists(const string& name);
	bool TryFindStructName(const string& name, AST_Struct_Definition& structDef);
};
