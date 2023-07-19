#include "IR.h"
//
//
//void IR::ConvertToIR()
//{
//	//IR_statements = AST->ConvertStatementToIR(state);
//}

void IR_Scope::EnterScope()
{
	++scopeIndex;
	variableMapping.push_back(unordered_map<string, IR_Value>());
	structMapping.push_back(unordered_map<string, StructDefinition>());
}
void IR_Scope::ExitScope()
{
	++scopeIndex;
	variableMapping.pop_back();
	structMapping.pop_back();
}

IR_Scope::IR_Scope()
{
	EnterScope();
}

IR::IR()
{
	IR_statements = vector<unique_ptr<IR_Statement>>();
}


IR_State::IR_State()
{
	//This is what the RAX or XMM0 register would be in x64
	functionReturnValue = IR_Value(IR_INT, IR_VARIABLE, 8, 0, true, "");
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
FunctionDefinition IR_Scope::FindFunction(string name)
{
	return functionMapping.at(name);
}

