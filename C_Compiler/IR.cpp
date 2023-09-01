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
	add_statement(std::make_shared<IR_ScopeStart>());
	
}
void IR::ExitScope()
{
	++state.scope.scopeIndex;
	state.scope.variableMapping.pop_back();
	state.scope.structMapping.pop_back();

	//TODO: see if this is needed
	add_statement(std::make_shared<IR_ScopeEnd>());

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

void IR::DetermineRegisterStatusOfOperand(IR_Operand& op, unordered_map<int, int>& nonRegisterVariableMap)
{
	if (op.useMemoryAddress)
	{
		nonRegisterVariableMap[op.value.varIndex] = op.value.byteSize;
	}
}


struct StatementVariableUse
{
	int lineNum = -1;
	int varIdx = 0;
	bool copy = false;

	StatementVariableUse(int lineNum, int varIdx) : lineNum(lineNum), varIdx(varIdx) {}
};

//This function doubles the statements within loops so that when trying to find the next line num of a variable
//it can look behind itself (if inside a loop)
void RecursivelyDoubleLoops(vector<StatementVariableUse>& out, vector<StatementVariableUse>& v, int& i)
{
	if (i >= v.size())
	{
		return;
	}

	int varIdx = v.at(i).varIdx;
	if (varIdx == 0)
	{
		RecursivelyDoubleLoops(out, v, ++i);
	}
	else if (varIdx > 0)
	{
		out.push_back(v.at(i));
		RecursivelyDoubleLoops(out, v, ++i);
	}
	else if (varIdx == -1)
	{
		vector<StatementVariableUse> doubledVector;
		RecursivelyDoubleLoops(doubledVector, v, ++i);
		out.insert(out.end(), doubledVector.begin(), doubledVector.end());
		out.insert(out.end(), doubledVector.begin(), doubledVector.end());

		RecursivelyDoubleLoops(out, v, i);
	}
	else
	{
		++i;
	}

}



IR_VariableData IR::ComputeIRVariableData()
{
	IR_VariableData varData;
	//unordered_map<int, int> nonRegisterVariableMap;

	for (auto& func : this->functions)
	{
		varData.nonRegisterVariables[func.functionName] = unordered_map<int, int>();
		varData.functionDefinitions[func.functionName] = IR_FunctionDef();

		vector<StatementVariableUse> variableUses;
		for (int lineNum = 0; lineNum < func.IR_statements.size(); ++lineNum)
		{
			const auto& statement = func.IR_statements.at(lineNum);
			IR_StatementType statementType = statement->GetType();

			switch (statementType)
			{
				case _IR_ASSIGN:
				{
					IR_Assign* assign = dynamic_cast<IR_Assign*>(statement.get());
					DetermineRegisterStatusOfOperand(assign->dest, varData.nonRegisterVariables.at(func.functionName));
					DetermineRegisterStatusOfOperand(assign->source, varData.nonRegisterVariables.at(func.functionName));

					variableUses.push_back(StatementVariableUse(lineNum, assign->dest.value.varIndex));
					variableUses.push_back(StatementVariableUse(lineNum, assign->source.value.varIndex));

					break;
				}
				case _IR_FUNCTION_ARG_ASSIGN:
				{
					IR_FunctionArgAssign* funcArgAssign = dynamic_cast<IR_FunctionArgAssign*>(statement.get());
					DetermineRegisterStatusOfOperand(funcArgAssign->value, varData.nonRegisterVariables.at(func.functionName));

					variableUses.push_back(StatementVariableUse(lineNum, funcArgAssign->value.value.varIndex));

					break;
				}
				case _IR_COMPARE:
				{
					IR_Compare* compare = dynamic_cast<IR_Compare*>(statement.get());
					DetermineRegisterStatusOfOperand(compare->op1, varData.nonRegisterVariables.at(func.functionName));
					DetermineRegisterStatusOfOperand(compare->op2, varData.nonRegisterVariables.at(func.functionName));

					variableUses.push_back(StatementVariableUse(lineNum, compare->op1.value.varIndex));
					variableUses.push_back(StatementVariableUse(lineNum, compare->op2.value.varIndex));

					break;
				}
				case _IR_CONTINUOUS_MEMORY_INIT:
				{
					IR_ContinuousMemoryInit* memoryInit = dynamic_cast<IR_ContinuousMemoryInit*>(statement.get());
					varData.nonRegisterVariables.at(func.functionName)[memoryInit->varIdx] = memoryInit->byteNum;
					break;
				}
				case _IR_LOOP_START:
				{
					variableUses.push_back(StatementVariableUse(lineNum, -1));
					break;
				}
				case _IR_LOOP_END:
				{
					variableUses.push_back(StatementVariableUse(lineNum, -2));
					break;
				}
				case _IR_FUNCTION_LABEL:
				{
					IR_FunctionLabel* functionLabel = dynamic_cast<IR_FunctionLabel*>(statement.get());

					IR_FunctionDef funcDef;
					funcDef.args = functionLabel->args;
					funcDef.functionName = func.functionName;
					funcDef.returnVar = functionLabel->returnValue;

					for (const IR_Value& arg : funcDef.args)
					{
						variableUses.push_back(StatementVariableUse(lineNum, arg.varIndex));
					}
					varData.functionDefinitions[functionLabel->functionName] = std::move(funcDef);

					
				}

			}
		}

		vector<StatementVariableUse> out;


		for (auto v : variableUses)
		{
			std::cout << v.varIdx << ", ";
		}
		std::cout << "\n______________\n";

		int l = 0;
		RecursivelyDoubleLoops(out, variableUses, l);

		for (auto v : out)
		{
			std::cout << v.varIdx << ", ";
		}

		unordered_map<int, vector<int>> variableLineMapping;
		map<int, int> normalIndexToDoubledIndexMapping;

		for (int i = 0; i < out.size(); ++i)
		{
			StatementVariableUse& var = out.at(i);

			variableLineMapping[var.varIdx].push_back(i);

			if (normalIndexToDoubledIndexMapping.find(var.lineNum) == normalIndexToDoubledIndexMapping.end())
			{
				normalIndexToDoubledIndexMapping[var.lineNum] = i;
			}
		}

		//reverse line mapping so you can just pop from end when the current index is greater than the back index
		for (auto& pair : variableLineMapping)
		{
			std::reverse(pair.second.begin(), pair.second.end());
		}

		varData.normalIndexToDoubledIndexMapping[func.functionName] = std::move(normalIndexToDoubledIndexMapping);
		varData.variableLineMapping[func.functionName] = std::move(variableLineMapping);
	}

	return varData;

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

