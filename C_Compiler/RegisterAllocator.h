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

	void SetRegister(REGISTER reg, int varIdx);
	void Reset();
};