#pragma once
#include "Parser.h"
#include "IR.h"
#include "RegisterAllocator.h"
#include "IR_Optimizations.h"

struct IR_CodeGen
{
	IR irState;
	IR_Scope scope;
	shared_ptr<StatementGroup> statements;

	IR_Optimizations optimizationSettings;

	IR_CodeGen(shared_ptr<StatementGroup> statements);

	void ConvertToIR();

	void PrintIR();

	void Optimize();

	void AllocateRegisters();
};