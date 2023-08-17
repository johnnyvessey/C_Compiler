#pragma once
#include "Parser.h"
#include "IR.h"
#include "RegisterAllocator.h"

struct IR_Optimizations
{

	bool optimizeJumps = true;

	void RemoveNoOps(vector<shared_ptr<IR_Statement>>& irStatements);

	void OptimizeJumps(vector<shared_ptr<IR_Statement>>& irStatements);

	void RemoveJumpToNextLine(vector<shared_ptr<IR_Statement>>& irStatements);
	void RemoveConsecutiveJumps(vector<shared_ptr<IR_Statement>>& irStatements);
	void RemoveUnusedLabels(vector<shared_ptr<IR_Statement>>& irStatements);

};

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