#include "x64_State.h"

const vector<REGISTER> x64_State::_calleeSavedRegisters({ R12, R13, R14, R15, RBX, RSP, RBP });

//reserve R15 and XMM15 for bringing spilled registers back to register temporarily
const vector<REGISTER> x64_State::_usableIntCalleeSavedRegisters({RDI, RSI, RDX, RCX, R8, R9, R10, R11, R12, R13, R14, RAX});
const vector<REGISTER> x64_State::_usableFloatCalleeSavedRegisters({XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7, XMM8, XMM9, XMM10, XMM11, XMM12, XMM13, XMM14, XMM0});

//TODO: figure out how this function should work
//should it be called on IR_Operands and allocate registers if needed
//or should that logic be in a higher up function that decides whether an allocation is needed or not
//OperandAsm x64_State::AllocateRegister(IR_Value value, REGISTER specificRegister)
//{
//	if (specificRegister == _NONE)
//	{
//		//if (this->irVariableData.nonRegisterVariables.find(value.varIndex) != this->irVariableData.nonRegisterVariables.end())
//		//{
//		//	int offset = this->registerAllocator.memoryVariableMapping.memoryOffsetMapping.at(value.varIndex);
//		//	return OperandAsm::CreateRSPOffsetOperand(offset);
//		//}
//		//else {
//			
//			const vector<REGISTER>& availableRegisters = value.type == IR_INT ? _usableIntCalleeSavedRegisters : _usableFloatCalleeSavedRegisters;
//			for (const REGISTER& reg : availableRegisters)
//			{
//				if (this->registerAllocator.registerMapping.mapping.at((int)reg).variableIndex == 0)
//				{
//					this->registerAllocator.registerMapping.mapping.at((int)reg) = RegisterVariableGroup(value.varIndex);
//
//					OperandAsm operand;
//					operand.reg = RegisterAsm(reg);
//					operand.type = ASM_REG;
//
//					return operand;
//				}
//			}
//			
//		
//		//}
//		
//
//		//spill register because no open registers were found
//
//		
//	}
//	else { //specific register needs to be allocated, such as for return value or division
//		//if value is taken, figure out how to spill it in case of control flow (value is spileed in one branch but not other...)
//		//could just reserve RAX and XMM0 to only be used to return values
//		//complicated b/c IDIV instruction uses RDX as well...
//	}
//
//
//	return OperandAsm();
//}
//
void x64_State::EvictExpiredVariables()
{
	for (int reg = 0; reg < NUM_REGISTERS; ++reg)
	{
		//if register is assigned to variable
		int varIdx = this->registerAllocator.registerMapping.mapping.at(reg).variableIndex;
		if (varIdx != 0)
		{
			int lineNumInDoubledIndices = this->irVariableData.currentNormalIndexToDoubledIndexMapping->at(this->lineNum);
			while (!this->irVariableData.currentLineMapping->empty() &&
				lineNumInDoubledIndices >= this->irVariableData.currentLineMapping->at(varIdx).back())
			{
				this->irVariableData.currentLineMapping->at(varIdx).pop_back();
			}

			if (this->irVariableData.currentLineMapping->empty())
			{
				//clear out variable from mapping if it is never used again
				this->registerAllocator.registerMapping.mapping.at(reg).variableIndex = 0;
			}

		}
	}

}


REGISTER x64_State::FindFurthestAwayRegisterInMappingUsed()
{
	int maxVarOccurrence = -1;
	int regToEvict = -1;

	for (int reg = 0; reg < NUM_REGISTERS; ++reg)
	{
		//if register is assigned to variable
		int varIdx = this->registerAllocator.registerMapping.mapping.at(reg).variableIndex;
		if (varIdx != 0)
		{
			int nextVarOccurrence = this->irVariableData.currentLineMapping->at(varIdx).back();

			if (nextVarOccurrence > maxVarOccurrence)
			{
				maxVarOccurrence = nextVarOccurrence;
				regToEvict = reg;
			}
		}
	}

	return (REGISTER)regToEvict;
}


OperandAsm x64_State::AllocateRegister(IR_Value value, REGISTER specificRegister)
{
	if (specificRegister == _NONE)
	{
		//if (this->irVariableData.nonRegisterVariables.find(value.varIndex) != this->irVariableData.nonRegisterVariables.end())
		//{
		//	int offset = this->registerAllocator.memoryVariableMapping.memoryOffsetMapping.at(value.varIndex);
		//	return OperandAsm::CreateRSPOffsetOperand(offset);
		//}
		//else {
			
			const vector<REGISTER>& availableRegisters = value.type == IR_INT ? _usableIntCalleeSavedRegisters : _usableFloatCalleeSavedRegisters;
			for (const REGISTER& reg : availableRegisters)
			{
				if (this->registerAllocator.registerMapping.mapping.at((int)reg).variableIndex == 0)
				{
					this->registerAllocator.registerMapping.mapping.at((int)reg) = RegisterVariableGroup(value.varIndex);

					return OperandAsm::CreateRegisterOperand(reg);
				}
			}

			//TODO: make sure to set register mapping as not-modified after setting value in memory


			//spill register: one that will be used the farthest away
			REGISTER spilledRegister = FindFurthestAwayRegisterInMappingUsed();

			if (this->registerAllocator.registerMapping.mapping.at((int)spilledRegister).isModified)
			{
				//write back to its memory location
			}

			this->registerAllocator.registerMapping.mapping.at((int)spilledRegister) = RegisterVariableGroup(value.varIndex);

		
	}
	else { 

		OperandAsm registerOperand = OperandAsm::CreateRegisterOperand(specificRegister);
		
		if (this->registerAllocator.registerMapping.mapping.at((int)specificRegister).variableIndex != 0)
		{
			//spill
		}
	
		return registerOperand;
		
	}


	return OperandAsm();
}