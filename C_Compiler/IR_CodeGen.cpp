#include "IR_CodeGen.h"


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

	for (const shared_ptr<IR_Statement>& statement : irState.IR_statements)
	{
		std::cout << statement->ToString() << "\n";
	}
}

