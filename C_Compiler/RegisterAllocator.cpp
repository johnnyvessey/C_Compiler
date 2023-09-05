#include "RegisterAllocator.h"


void RegisterAllocator::SetRegister(REGISTER reg, int varIdx)
{
	registerMapping.regMapping.at((int)reg) = RegisterVariableGroup(varIdx, true);
}

void RegisterAllocator::Reset()
{
	usedRegisters = vector<int>(NUM_REGISTERS, 0);
	//clear register mapping
	for (int i = 0; i < REGISTER_SIZE; ++i)
	{
		registerMapping.regMapping.at(i).variableIndex = 0;
	}

	memoryVariableMapping.memoryOffsetMapping.clear();
	memoryVariableMapping.memoryOffsetMappingSpilledRegisters.clear();
	currentFramePointerOffset = 0;
}

void RegisterAllocator::SetInitialLabelMapping(int label, RegisterMapping regMapping)
{
	if (labelRegisterMapping.find(label) != labelRegisterMapping.end())
	{
		labelRegisterMapping[label].initialMapping = regMapping;
	}
	else {
		LabelRegisterMaps map;
		map.initialMapping = std::move(regMapping);
		labelRegisterMapping[label] = map;
	}
}
void RegisterAllocator::AddJumpLabelMapping(int label, RegisterMapping regMapping, int jumpStatementIndex)
{
	if (labelRegisterMapping.find(label) != labelRegisterMapping.end())
	{
		labelRegisterMapping[label].jumpMappings.push_back(JumpRegisterMapping(regMapping, jumpStatementIndex));
	}
	else {
		LabelRegisterMaps map;
		map.jumpMappings.push_back(JumpRegisterMapping(regMapping, jumpStatementIndex));
		labelRegisterMapping[label] = map;
	}

}



//void RegisterAllocator::AllocateRegisters(vector<IR_Function_Group>& functions)
//{
//	for (IR_Function_Group& function : functions)
//	{
//		AllocateRegistersForFunction(function);
//	}
//}
//
//void RegisterAllocator::AllocateRegistersForFunction(IR_Function_Group& function)
//{
//	//set up mapping to match input arguments to registers and memory locations
//}



/*
	Register Allocation for Control Flow:
	
	At every jump, take note of the current register mapping and add it the vector of mappings on the label it jumps to.
	Also, every time you reach a label normally by iterating through statements, set the main mapping of that label to the current mapping.
	Find the intersection of all mappings at a label to determine what the register mapping should be going forward.
	Once you know the finalized register mapping at a label, do load/store operations before statements that jump to that label so that the register
		mapping matches the one at the label.
	For loops, be careful....





*/