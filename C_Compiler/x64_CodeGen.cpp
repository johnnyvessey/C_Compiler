#include "x64_CodeGen.h"

x64_CodeGen::x64_CodeGen(IR& irState): irState(irState) {}

void x64_CodeGen::PrintCurrentRegisterMapping()
{
	//print registers
	std::cout << "Reg: {";
	for (int i = 0; i < NUM_REGISTERS; ++i)
	{
		const int varIndex = this->state.registerAllocator.registerMapping.mapping.at(i).variableIndex;
		if (varIndex != 0)
		{
			std::cout << REGISTER_STRING.at(i) << ": %" << varIndex << ", ";
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
		this->state.irVariableData.currentFunctionVariableRanges = &(this->state.irVariableData.variableRanges.at(func.functionName));
		this->state.statements.push_back(StatementAsm(x64_FUNCTION_PROC, func.functionName));

		//function body
		for (int i = 0; i < func.IR_statements.size(); ++i)
		{
			state.lineNum = i;
			func.IR_statements.at(i)->ConvertToX64(state);
			std::cout << func.IR_statements.at(i)->ToString() << "\n";

			this->state.ExpireOldIntervals();
			PrintCurrentRegisterMapping();	
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