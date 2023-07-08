#include "Scope.h"

Scope::Scope() {
	scope = vector<ScopeLevel>(1, ScopeLevel());
}

bool Scope::TryFindVariable(const string& name, Variable& var)
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

bool Scope::TryFindFunction(const string& name, FunctionDefinition& func)
{
	if (functionScope.find(name) != functionScope.end())
	{
		func = functionScope.at(name);
		return true;
	}


	return false;
}

bool Scope::FunctionNameExists(const string& name)
{
	return functionScope.find(name) != functionScope.end();
}

bool Scope::TryFindStructName(const string& name, StructDefinition& structDef)
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
