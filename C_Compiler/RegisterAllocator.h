#pragma once
#include <vector>
#include "Variable.h"

using std::vector;

struct RegisterVariableGroup
{
	//REGISTER reg;
	int variableIndex = 0;
	bool isModified = false;

	//TODO: figure out if register size is necessary (i.e. RAX, EAX, AL...)
	RegisterVariableGroup();
	RegisterVariableGroup(int varIndex);
};

struct RegisterMapping
{
	vector<RegisterVariableGroup> mapping;
	RegisterMapping();
	void SetRegister(int reg, int variable);
	bool FindRegisterOfVariable(int variable, int& reg);
};

struct MemoryVariableMapping
{
	//TODO: clear this at start of every function
	unordered_map<int, int> memoryOffsetMapping; //maps varIndex to offset from RSP pointer (positive for arguments, negative for local variables)
	unordered_map<int, int> memoryOffsetMappingSpilledRegisters; //maps varIndex to offset from RSP pointer (positive for arguments, negative for local variables)

};

struct RegisterAllocator
{
	RegisterMapping registerMapping;
	MemoryVariableMapping memoryVariableMapping;
	int currentStackPointerOffset;
};