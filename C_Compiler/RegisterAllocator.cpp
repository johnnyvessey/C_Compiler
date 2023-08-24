#include "RegisterAllocator.h"




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