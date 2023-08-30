#include "x64_CodeGen.h"

x64_CodeGen::x64_CodeGen(IR& irState): irState(irState) {}

void x64_CodeGen::PrintNonRegisterIRVariables()
{
	std::cout << "Non register variables: \n{";

	//loop through each function name
	for (const auto& funcPair : state.irVariableData.functionDefinitions)
	{
		for (const auto& pair : state.irVariableData.nonRegisterVariables.at(funcPair.first))
		{
			std::cout << "%" << pair.first << ": " << pair.second << " bytes, ";
		}
	}
	std::cout << "}\n";

}

void x64_CodeGen::PrintVariableRanges()
{
	std::cout << "Variable Ranges: \n";

	for (const auto& funcNameLineMappingPair : state.irVariableData.variableLineMapping)
	{
		std::cout << funcNameLineMappingPair.first << ":\n{";
		for (const auto& pair : state.irVariableData.normalIndexToDoubledIndexMapping.at(funcNameLineMappingPair.first))
		{
			std::cout << pair.first << ": " << pair.second << ", ";
		}

		std::cout << "}\n\n";
		for (const auto& varIdxRangePair : funcNameLineMappingPair.second)
		{
			std::cout << "%" << varIdxRangePair.first << ": {";
			for (const auto& v : varIdxRangePair.second)
			{
				std::cout << v << ", ";
			}
			std::cout << "}\n";
		}
		std::cout << "-----\n";
	}
}

void x64_CodeGen::PrintCurrentRegisterMapping()
{
	//print registers
	std::cout << "Reg: {";
	for (int i = 0; i < NUM_REGISTERS; ++i)
	{
		const int varIndex = this->state.registerAllocator.registerMapping.regMapping.at(i).variableIndex;
		if (varIndex != 0)
		{
			std::cout << RegisterString::registerStringMapping.at(i) << ": %" << varIndex << ", ";
		}
	}
	std::cout << "}\n";

	//print variables stored in memory with offset from stack pointer
	std::cout << "Mem: {";
	for (const auto& pair : this->state.registerAllocator.memoryVariableMapping.memoryOffsetMapping)
	{
		std::cout << "RSP + " << pair.second << ": %" << pair.first << ", ";
	}
	std::cout << "}\n------\n";
}

void x64_CodeGen::GenerateCode()
{
	//get info about IR variables to use in register allocation algorithm
	this->state.irVariableData = irState.ComputeIRVariableData();

	//initialize scope stack
	this->state.irVariableData.irScopeStack.push_back(vector<int>());

	this->PrintNonRegisterIRVariables();
	this->PrintVariableRanges();

	std::cout << "------\n\n\n\n";

	//global data
	this->state.statements.push_back(StatementAsm(x64_DATA_SECTION));
	for (int i = 0; i < this->irState.floatLiteralGlobals.size(); ++i)
	{
		//TODO: generate this
		//add GLOBAL_VARIABLE statements
	}

	//functions
	this->state.statements.push_back(StatementAsm(x64_CODE_SECTION));

	for (const auto& func : this->irState.functions)
	{
		//reset all register allocation and stored variables
		this->state.registerAllocator.Reset();

		//set up address variables on stack
		this->state.CreateStackSpaceForVariables(func.functionName);

		//assign registers and stack space based on function arguments
		this->state.SetUpFunctionVariableMappings(func.functionName);
		
		//set current line and index mappings to use for register allocation
		this->state.irVariableData.currentLineMapping = &(this->state.irVariableData.variableLineMapping).at(func.functionName);
		this->state.irVariableData.currentNormalIndexToDoubledIndexMapping = &(this->state.irVariableData.normalIndexToDoubledIndexMapping.at(func.functionName));

		this->state.statements.push_back(StatementAsm(x64_FUNCTION_PROC, func.functionName));

		//function body
		for (int i = 0; i < func.IR_statements.size(); ++i)
		{
			state.lineNum = i;
			func.IR_statements.at(i)->ConvertToX64(state);
			//std::cout << func.IR_statements.at(i)->ToString() << "\n";

			this->state.EvictExpiredVariables();
			//PrintCurrentRegisterMapping();	
		}

		this->state.statements.push_back(StatementAsm(x64_FUNCTION_END, func.functionName));
	}

	this->state.statements.push_back(StatementAsm(x64_CODE_END));

}

string x64_CodeGen::CodeToString()
{
	stringstream ss;
	for (const StatementAsm& statement : this->state.statements)
	{	
		ss << statement.ToString() << "\n";
	}

	std::cout << ss.str();
	return ss.str();
}