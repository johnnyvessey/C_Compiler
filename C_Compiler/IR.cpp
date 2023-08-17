#include "IR.h"

//
//
//void IR::ConvertToIR()
//{
//	//IR_statements = AST->ConvertStatementToIR(state);
//}

void IR::EnterScope()
{
	++state.scope.scopeIndex;
	state.scope.variableMapping.push_back(unordered_map<string, IR_Value>());
	state.scope.structMapping.push_back(unordered_map<string, StructDefinition>());

	//TODO: see if this is needed
	//IR_statements.push_back(std::make_shared<IR_ScopeStart>());
	
}
void IR::ExitScope()
{
	++state.scope.scopeIndex;
	state.scope.variableMapping.pop_back();
	state.scope.structMapping.pop_back();

	//TODO: see if this is needed
	//IR_statements.push_back(std::make_shared<IR_ScopeEnd>());

}

void IR::EnterFunction()
{
	add_statement(make_shared<IR_FunctionStart>());
	EnterScope();
}
void IR::ExitFunction()
{
	ExitScope();
	add_statement(make_shared<IR_FunctionEnd>());
}
IR_Scope::IR_Scope()
{
	variableMapping.push_back(unordered_map<string, IR_Value>());
	structMapping.push_back(unordered_map<string, StructDefinition>());

}

IR::IR()
{
	//IR_statements = vector<shared_ptr<IR_Statement>>();
}

IR_Function_Group::IR_Function_Group(string functionName) : functionName(functionName) {}


IR_State::IR_State() : functionReturnValueInt(IR_Value(IR_INT, IR_VARIABLE, 8, 0, true, "", IR_RETURN_INT)), 
	functionReturnValueFloat(IR_Value(IR_FLOAT, IR_VARIABLE, 8, 0, true, "", IR_RETURN_FLOAT)),
	functionReturnValueStructPointer(IR_Value(IR_INT, IR_VARIABLE, 8, 0, true, "", IR_RETURN_STACK, 1, IR_STRUCT))
{
	flags = IR_Value(IR_INT, IR_VARIABLE, 1, 0, true, "", IR_FLAGS);
}

IR_Value IR_Scope::FindVariable(string name)
{
	for (int i = variableMapping.size() - 1; i >= 0; --i)
	{
		if (variableMapping[i].find(name) != variableMapping[i].end())
		{
			return variableMapping[i].at(name);
		}
	}
}

StructDefinition IR_Scope::FindStruct(string name)
{
	for (int i = structMapping.size() - 1; i >= 0; --i)
	{
		if (structMapping[i].find(name) != structMapping[i].end())
		{
			return structMapping[i].at(name);
		}
	}
}
IR_FunctionLabel IR_Scope::FindFunction(string name)
{
	return functionMapping.at(name);
}

