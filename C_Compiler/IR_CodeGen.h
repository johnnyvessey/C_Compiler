#pragma once
#include "Parser.h"
#include "IR.h"


struct IR_CodeGen
{
	IR irState;
	IR_Scope scope;
	shared_ptr<StatementGroup> statements;

	vector<IR_Statement> irStatements;

	IR_CodeGen(shared_ptr<StatementGroup> statements);

	void ConvertToIR();

	void PrintIR();
};