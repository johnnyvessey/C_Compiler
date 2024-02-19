#pragma once
#include "Parser.h"
#include "IR.h"

struct IR_Optimizations
{

	bool optimizeJumps = true;

	void RemoveNoOps(vector<shared_ptr<IR_Statement>>& irStatements);

	void OptimizeJumps(vector<shared_ptr<IR_Statement>>& irStatements);

	void RemoveJumpToNextLine(vector<shared_ptr<IR_Statement>>& irStatements);
	void RemoveConsecutiveJumps(vector<shared_ptr<IR_Statement>>& irStatements);
	void RemoveUnusedLabels(vector<shared_ptr<IR_Statement>>& irStatements);
	void ConstantPropagationAndDCE(vector<shared_ptr<IR_Statement>>& irStatements, const unordered_map<int,int> nonRegisterVariables);
	void RemoveUnusedVariables(vector<shared_ptr<IR_Statement>>& irStatements, const unordered_map<int, int> nonRegisterVariables);
};