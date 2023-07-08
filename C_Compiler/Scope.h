#pragma once

#include "Variable.h"

using namespace VariableNamespace;

/*
SCOPE of variables:
- vector functioning as stack
- push new scope object to stack when entering, pop when leaving scope (don't care about variables once we're out of scope)
- iterate through vector backwards to find the variable/function/struct name
*/
struct ScopeLevel {
	unordered_map<string, Variable> variables;
	unordered_map<string, StructDefinition> structs;
};

struct Scope {
	vector<ScopeLevel> scope;
	unordered_map<string, FunctionDefinition> functionScope; //functions cannot be declared within functions / inner scope

	Scope();
	bool TryFindVariable(const string& name, Variable& var);
	bool TryFindFunction(const string& name, FunctionDefinition& func);
	bool FunctionNameExists(const string& name);
	bool TryFindStructName(const string& name, StructDefinition& structDef);
};
