#include "IR_CodeGen.h"
#include <unordered_set>

using std::unordered_set;

void IR_CodeGen::ConvertToIR()
{
	statements->ConvertStatementToIR(irState);
}

IR_CodeGen::IR_CodeGen(shared_ptr<StatementGroup> statements) : statements(statements) {}

void IR_CodeGen::PrintIR()
{
	//print struct definitions
	//print function definitions

	//print regular statements

	std::cout << ".data\n";
	for (IR_Value& value : irState.state.scope.globalVariables)
	{
		std::cout << "\t%v" << value.varIndex << " dq " << value.literalValue << "\n";
	}
	for (int i = 0; i < irState.floatLiteralGlobals.size(); ++i)
	{
		std::cout << "\tvar" << (i + 1) << " dq " << irState.floatLiteralGlobals.at(i) << "\n";
	}

	std::cout << "\n\n.code\n\n";

	for (const IR_Function_Group& function : irState.functions)
	{
		for (const shared_ptr<IR_Statement>& statement : function.IR_statements)
		{
			std::cout << statement->ToString() << "\n";
		}
		std::cout << "-----------\n";
	}

}


void IR_CodeGen::Optimize()
{

	//jump optimization pass
	for (IR_Function_Group& function : irState.functions)
	{
		optimizationSettings.OptimizeJumps(function.IR_statements);
	}

	//constant propogation and dead code elimination pass 
	IR_VariableData varData = irState.ComputeIRVariableData();
	for (IR_Function_Group& function : irState.functions)
	{
		optimizationSettings.ConstantPropagationAndDCE(function.IR_statements, varData.nonRegisterVariables.at(function.functionName));
	}

	
	//remove unused variables
	for (IR_Function_Group& function : irState.functions)
	{
		optimizationSettings.RemoveUnusedVariables(function.IR_statements, varData.nonRegisterVariables.at(function.functionName));
	}
}


void IR_CodeGen::AllocateRegisters()
{
	RegisterAllocator registerAllocator;
	//registerAllocator.AllocateRegisters(irState.functions);
}