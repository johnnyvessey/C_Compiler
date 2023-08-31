#pragma once
#include <vector>
#include "Variable.h"

using std::vector;

using namespace VariableNamespace;


struct RegisterAllocator
{
	RegisterMapping registerMapping;
	MemoryVariableMapping memoryVariableMapping;
	unordered_map<int, LabelRegisterMaps> labelRegisterMapping;
	int currentStackPointerOffset;

	int startFunctionStackPointerSubtractIndex = -1;
	vector<LoopLabel> startLoopLabelIndexes;

	void SetRegister(REGISTER reg, int varIdx);
	void Reset();

	void SetInitialLabelMapping(int label, RegisterMapping mapping);
	void AddJumpLabelMapping(int label, RegisterMapping mapping, int jumpStatementIdx);

};