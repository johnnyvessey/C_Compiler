#pragma once

#include "AST_Statement.h"

using namespace AST_Statement;

namespace ScopeNamespace
{
	/*
	SCOPE of variables:
	- vector functioning as stack
	- push new scope object to stack when entering, pop when leaving scope (don't care about variables once we're out of scope)
	- iterate through vector backwards to find the variable/function/struct name
	*/
	struct ScopeLevel {
		unordered_map<string, Variable> variables;
		unordered_map<string, Function> functions;
		unordered_map<string, AST_Struct_Definition> structs;
	};

	struct Scope {
		vector<ScopeLevel> scope;

		Scope() {
			scope = vector<ScopeLevel>(1, ScopeLevel());
		}

		bool TryFindVariable(const string& name, Variable& var)
		{
			for (int i = scope.size() - 1; i >= 0; --i)
			{
				if (scope[i].variables.find(name) != scope[i].variables.end())
				{
					var = scope[i].variables.at(name);
					return true;
				}
			}

			return false;
		}

		bool TryFindFunction(const string& name, Function& func)
		{
			for (int i = scope.size() - 1; i >= 0; --i)
			{
				if (scope[i].functions.find(name) != scope[i].functions.end())
				{
					func = scope[i].functions.at(name);
					return true;
				}
			}

			return false;
		}

		bool TryFindStructName(const string& name, AST_Struct_Definition& structDef)
		{
			for (int i = scope.size() - 1; i >= 0; --i)
			{
				if (scope[i].structs.find(name) != scope[i].structs.end())
				{
					structDef = scope[i].structs.at(name);
					return true;
				}
			}

			return false;
		}
	};
}