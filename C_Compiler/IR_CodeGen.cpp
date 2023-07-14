#include "IR_CodeGen.h"


void IR_CodeGen::ConvertToIR()
{
	statements->ConvertStatementToIR(irState, scope);
}

IR_CodeGen::IR_CodeGen(shared_ptr<StatementGroup> statements) : statements(statements) {}

void IR_CodeGen::PrintIR()
{
	//print struct definitions
	//print function definitions

	//print regular statements
}