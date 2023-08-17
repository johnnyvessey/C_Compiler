#pragma once
#include "IR.h"
#include <vector>
using std::vector;

struct RegisterVariableGroup
{
	//REGISTER reg;
	int variableIndex;
	bool isModified = false;

	//TODO: figure out if register size is necessary (i.e. RAX, EAX, AL...)
	RegisterVariableGroup();
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
	unordered_map<int, int> memoryOffsetMapping; //maps varIndex to offset from RSP pointer (positive for arguments, negative for local variables)
};

struct RegisterAllocator
{
	void AllocateRegisters(vector<IR_Function_Group>& functions);
	void AllocateRegistersForFunction(IR_Function_Group& function);

	RegisterMapping registerMapping;
	MemoryVariableMapping memoryVariableMapping;
	int currentStackPointerOffset;
};