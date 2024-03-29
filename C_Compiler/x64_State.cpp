#include "x64_State.h"

//const vector<REGISTER> x64_State::_calleeSavedRegisters({ R12, R13, R14, R15, RBX, RSP, RBP });
const vector<REGISTER> x64_State::_calleeSavedRegisters({ R12, R13, R14, R15, RBX });

//reserve R15 and XMM15 for bringing spilled registers back to register temporarily
const vector<REGISTER> x64_State::_usableIntRegisters({RDI, RSI, RCX, R8, R9, R10, R11, R12, R13, R14, R15, RBX, RDX, RAX});
//const vector<REGISTER> x64_State::_usableIntRegisters({R12, R13, R14, R15, RBX, RDX, RAX });

//const vector<REGISTER> x64_State::_usableIntCalleeSavedRegisters({RDI, RSI, RCX});

const vector<REGISTER> x64_State::_usableFloatRegisters({XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7, XMM8, XMM9, XMM10, XMM11, XMM12, XMM13, XMM14, XMM15, XMM0});
const vector<REGISTER> x64_State::_functionIntArguments({RDI, RSI, RDX, RCX, R8, R9});
const vector<REGISTER> x64_State::_functionFloatArguments({XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7});

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

void x64_State::CreateStackSpaceForVariables(const string& funcName)
{
	for (const auto& pair : irVariableData.nonRegisterVariables.at(funcName))
	{
		registerAllocator.currentFramePointerOffset -= pair.second;
		registerAllocator.memoryVariableMapping.memoryOffsetMapping[pair.first] = registerAllocator.currentFramePointerOffset;
	}
}

StatementAsm x64_State::SpillRegister(REGISTER reg, RegisterMapping& mapping)
{
	RegisterVariableGroup var = mapping.regMapping.at((int)reg);

	if (var.variableIndex == 0)
	{
		std::cout << "Variable doesn't exist at register";
		throw 0;
	}

	if (registerAllocator.memoryVariableMapping.memoryOffsetMappingSpilledRegisters.find(var.variableIndex) == 
		registerAllocator.memoryVariableMapping.memoryOffsetMappingSpilledRegisters.end())
	{
		registerAllocator.currentFramePointerOffset -= REGISTER_SIZE;
		registerAllocator.memoryVariableMapping.memoryOffsetMappingSpilledRegisters[var.variableIndex] = memoryOffset(registerAllocator.currentFramePointerOffset, true);
	}
	bool isFloat = (int)reg >= (int)XMM0;
	StatementAsm storeStatement(isFloat ? x64_MOVS: x64_MOV);
	storeStatement.firstOperand = OperandAsm::CreateOffsetOperand(registerAllocator.memoryVariableMapping.memoryOffsetMappingSpilledRegisters.at(var.variableIndex));
	storeStatement.secondOperand = OperandAsm::CreateRegisterOperand(reg);

	return storeStatement;
}

void x64_State::SetUpFunctionVariableMappings(const string& functionName)
{
	//set up register mapping and stack space above SP with extra function params
	IR_FunctionDef funcDef = irVariableData.functionDefinitions.at(functionName);

	int offset = 0;
	int intArgCount = 0;
	int floatArgCount = 0;

	for (const IR_Value& value : funcDef.args)
	{
		
		if (value.type == IR_INT && intArgCount < _functionIntArguments.size())
		{
			registerAllocator.registerMapping.SetRegister(_functionIntArguments.at(intArgCount), value.varIndex);
			++intArgCount;
		}
		else if (value.type == IR_FLOAT && floatArgCount < _functionFloatArguments.size())
		{
			registerAllocator.registerMapping.SetRegister(_functionFloatArguments.at(floatArgCount), value.varIndex);
			++floatArgCount;
		}
		else {
			registerAllocator.memoryVariableMapping.memoryOffsetMapping[value.varIndex] = offset; //offset from RBP
			offset += value.byteSize;

		}
	}


}



void x64_State::EvictExpiredVariables()
{
	for (int reg = 0; reg < NUM_REGISTERS; ++reg)
	{
		//if register is assigned to variable
		int varIdx = this->registerAllocator.registerMapping.regMapping.at(reg).variableIndex;

		//clear out temp variables (which are set to -1)
		if (varIdx < 0)
		{
			this->registerAllocator.registerMapping.regMapping.at(reg).variableIndex = 0;
		}
		else if (varIdx != 0)
		{
			//continue if there are no variables used on this line
			/*if (this->irVariableData.currentNormalIndexToDoubledIndexMapping->find(this->lineNum) ==
				this->irVariableData.currentNormalIndexToDoubledIndexMapping->end())
			{
				continue;
			}*/
			int lineNumInDoubledIndices = this->irVariableData.currentNormalIndexToDoubledIndexMapping->at(this->lineNum);
			while (this->irVariableData.currentLineMapping->at(varIdx).size() > 0 &&
				lineNumInDoubledIndices >= this->irVariableData.currentLineMapping->at(varIdx).back())
			{
				this->irVariableData.currentLineMapping->at(varIdx).pop_back();
			}

			if (this->irVariableData.currentLineMapping->at(varIdx).empty())
			{
				//clear out variable from mapping if it is never used again
				this->registerAllocator.registerMapping.regMapping.at(reg).variableIndex = 0;
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
		int varIdx = this->registerAllocator.registerMapping.regMapping.at(reg).variableIndex;
		if (varIdx > 0)
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


REGISTER x64_State::AllocateRegister(IR_Value value, REGISTER specificRegister)
{
	if (specificRegister == _NONE)
	{			
		bool notYetSpilled = (registerAllocator.memoryVariableMapping.memoryOffsetMappingSpilledRegisters.find(value.varIndex) ==
			registerAllocator.memoryVariableMapping.memoryOffsetMappingSpilledRegisters.end());

		//if variable is already assigned to a register, return it
		int endReg;
		if (this->registerAllocator.registerMapping.FindRegisterOfVariable(value.varIndex, endReg))
		{
			this->registerAllocator.usedRegisters.at(endReg) = 1;
			return (REGISTER)endReg;
		}

		const vector<REGISTER>& availableRegisters = value.type == IR_INT ? _usableIntRegisters : _usableFloatRegisters;
		for (const REGISTER& reg : availableRegisters)
		{
			if (this->registerAllocator.registerMapping.regMapping.at((int)reg).variableIndex == 0)
			{
				//if register had been spilled to memory already, it is not modified
				//load value from memory into the register if it has been spilled already
				if (!notYetSpilled)
				{
					StatementAsm loadRegister(value.type == IR_INT ? x64_MOV : x64_MOVS);
					loadRegister.firstOperand = OperandAsm::CreateRegisterOperand(reg);
					loadRegister.secondOperand = OperandAsm::CreateOffsetOperand(this->registerAllocator.memoryVariableMapping.
						memoryOffsetMappingSpilledRegisters.at(value.varIndex));
					statements.push_back(std::move(loadRegister));

				}

				this->registerAllocator.registerMapping.regMapping.at((int)reg) = RegisterVariableGroup(value.varIndex, notYetSpilled);

				
				this->registerAllocator.usedRegisters.at(reg) = 1;
				return reg;
			}
		}

		//spill register: one that will be used the farthest away
		REGISTER spilledRegister = FindFurthestAwayRegisterInMappingUsed();

		statements.push_back(SpillRegister(spilledRegister, registerAllocator.registerMapping));

		if (!notYetSpilled)
		{
			StatementAsm loadRegister(value.type == IR_INT ? x64_MOV : x64_MOVS);
			loadRegister.firstOperand = OperandAsm::CreateRegisterOperand(spilledRegister);
			loadRegister.secondOperand = OperandAsm::CreateOffsetOperand(this->registerAllocator.memoryVariableMapping.
				memoryOffsetMappingSpilledRegisters.at(value.varIndex));

			statements.push_back(std::move(loadRegister));
		}

		this->registerAllocator.registerMapping.regMapping.at((int)spilledRegister) = RegisterVariableGroup(value.varIndex, notYetSpilled);

		this->registerAllocator.usedRegisters.at(spilledRegister) = 1;
		//return OperandAsm::CreateRegisterOperand(spilledRegister);
		return spilledRegister;
		
	}
	else { 
		
		if (this->registerAllocator.registerMapping.regMapping.at((int)specificRegister).variableIndex != value.varIndex)
		{
			statements.push_back(SpillRegister(specificRegister, registerAllocator.registerMapping));
		}
	
		this->registerAllocator.registerMapping.regMapping.at((int)specificRegister) = RegisterVariableGroup(value.varIndex, true);

		this->registerAllocator.usedRegisters.at(specificRegister) = 1;

		return specificRegister;
		
	}

}

REGISTER x64_State::FindOpenRegister(OperandAsm operand, bool isFloat, bool shouldLoadRegister, bool xmmwordSize)
{
	const vector<REGISTER>& availableRegisters = isFloat ? _usableFloatRegisters : _usableIntRegisters;
	for (const REGISTER& reg : availableRegisters)
	{
		if (this->registerAllocator.registerMapping.regMapping.at((int)reg).variableIndex == 0)
		{
			if (shouldLoadRegister)
			{
				StatementAsmType type = xmmwordSize ? x64_MOVUPS : (isFloat ? x64_MOVS : x64_MOV);
				StatementAsm loadRegister(type);
				loadRegister.firstOperand = OperandAsm::CreateRegisterOperand(reg);
				loadRegister.secondOperand = operand;
				loadRegister.secondOperand.xmmwordSize = xmmwordSize;

				statements.push_back(std::move(loadRegister));
			}
			this->registerAllocator.registerMapping.regMapping.at((int)reg) = RegisterVariableGroup(-1, true);

			return reg;
		}
	}

	return _NONE;
}

REGISTER x64_State::AllocateTempRegister(OperandAsm operand, bool isFloat, bool shouldLoadRegister, bool xmmwordSize)
{
	REGISTER foundReg = FindOpenRegister(operand, isFloat, shouldLoadRegister, xmmwordSize);
	if (foundReg != _NONE)
	{
		this->registerAllocator.usedRegisters.at(foundReg) = 1;
		return foundReg;
	}

	//if there are no registers on the first try, evict the expired variables then try again
	//this will affect struct copying and allow more registers to be used without there being a risk of unnecessary spilling
	EvictExpiredVariables();

	REGISTER foundRegTry2 = FindOpenRegister(operand, isFloat, shouldLoadRegister, xmmwordSize);
	if (foundRegTry2 != _NONE)
	{
		this->registerAllocator.usedRegisters.at(foundRegTry2) = 1;
		return foundRegTry2;
	}

	REGISTER spilledRegister = FindFurthestAwayRegisterInMappingUsed();
	statements.push_back(SpillRegister(spilledRegister, registerAllocator.registerMapping));

	if (shouldLoadRegister)
	{
		StatementAsm loadRegister(isFloat == IR_INT ? x64_MOV : x64_MOVS);
		loadRegister.firstOperand = OperandAsm::CreateRegisterOperand(spilledRegister);
		loadRegister.secondOperand = operand;

		statements.push_back(std::move(loadRegister));
	}
	

	this->registerAllocator.registerMapping.regMapping.at((int)spilledRegister) = RegisterVariableGroup(-1, true);
	this->registerAllocator.usedRegisters.at(spilledRegister) = 1;

	return spilledRegister;
}

void x64_State::SpillRegisterIfChanged(RegisterMapping& mapping, int reg, int jumpStatementIdx)
{
	RegisterVariableGroup& regGroup = mapping.regMapping.at(reg);

	int lineNumInDoubledIndices = this->irVariableData.currentNormalIndexToDoubledIndexMapping->at(this->lineNum);
	if (regGroup.variableIndex != 0 && 
		//only spill if the variable will be used again after the label
		!this->irVariableData.currentLineMapping->at(regGroup.variableIndex).empty() && 
		lineNumInDoubledIndices <= this->irVariableData.currentLineMapping->at(regGroup.variableIndex).at(0))
	{
		if (regGroup.isModified)
		{
			if (jumpStatementIdx == -1)
			{
				statements.push_back(SpillRegister((REGISTER)reg, mapping));
			}
			else {
				statements.at(jumpStatementIdx).preStatements.push_back(SpillRegister((REGISTER)reg, mapping));
			}
		}

		mapping.regMapping.at(reg).variableIndex = 0;
	}
}

void x64_State::SetRegisterMappingToIntersectionOfMappings(int labelIdx)
{

	vector<JumpRegisterMapping>& mappings = registerAllocator.labelRegisterMapping.at(labelIdx).jumpMappings;
	RegisterMapping finalMapping = mappings.at(0).jumpRegMapping;

	for (int mappingIdx = 1; mappingIdx < mappings.size(); ++mappingIdx)
	{
		JumpRegisterMapping& currentMapping = mappings.at(mappingIdx);
		for (int reg = 0; reg < NUM_REGISTERS; ++reg)
		{
			if (finalMapping.regMapping.at(reg).variableIndex != currentMapping.jumpRegMapping.regMapping.at(reg).variableIndex)
			{
				SpillRegisterIfChanged(finalMapping, reg, mappings.at(0).jumpStatementIndex);
				SpillRegisterIfChanged(currentMapping.jumpRegMapping, reg, currentMapping.jumpStatementIndex);
			}
		}
	}

	
	this->registerAllocator.registerMapping = finalMapping;
}

void x64_State::MatchRegisterMappingsToIntialMapping(int labelIdx, int labelStatementIdx)
{
	RegisterMapping& initialMapping = registerAllocator.labelRegisterMapping.at(labelIdx).initialMapping;
	vector<JumpRegisterMapping>& mappings = registerAllocator.labelRegisterMapping.at(labelIdx).jumpMappings;
	unordered_set<int> variablesToReloadAtStartOfLoop;

	//store variables that don't match initial mapping
	for(int i = 0; i < mappings.size(); ++i)
	{ 
		JumpRegisterMapping& currentMapping = mappings.at(i);
		for (int reg = 0; reg < NUM_REGISTERS; ++reg)
		{
			if (initialMapping.regMapping.at(reg).variableIndex != 0 && 
				initialMapping.regMapping.at(reg).variableIndex != currentMapping.jumpRegMapping.regMapping.at(reg).variableIndex)
			{
				SpillRegisterIfChanged(currentMapping.jumpRegMapping, reg, currentMapping.jumpStatementIndex);
				variablesToReloadAtStartOfLoop.insert(initialMapping.regMapping.at(reg).variableIndex);
			}
		}

	}

	//add load statements after start loop label for changed variables
	for (int reg = 0; reg < NUM_REGISTERS; ++reg)
	{
		int varIdx = initialMapping.regMapping.at(reg).variableIndex;
		if (varIdx != 0 && (variablesToReloadAtStartOfLoop.find(varIdx) != variablesToReloadAtStartOfLoop.end()))
		{
			bool isFloat = (int)reg >= (int)XMM0;
			StatementAsm loadStatement(isFloat ? x64_MOVS : x64_MOV);

			loadStatement.firstOperand = OperandAsm::CreateRegisterOperand((REGISTER)reg);
			loadStatement.secondOperand = OperandAsm::CreateOffsetOperand(registerAllocator.memoryVariableMapping.
				memoryOffsetMappingSpilledRegisters.at(varIdx));
			this->statements.at(labelStatementIdx).postStatements.push_back(loadStatement);

			//before the loop, store the intial register values in the memory location, so the values will be valid for the first iteration of the loop
			StatementAsm storeStatementPreLoop(isFloat ? x64_MOVS : x64_MOV);
			storeStatementPreLoop.firstOperand = loadStatement.secondOperand;
			storeStatementPreLoop.secondOperand = loadStatement.firstOperand;

			this->statements.at(labelStatementIdx).preStatements.push_back(std::move(storeStatementPreLoop));


		}
	}

}

void x64_State::StructCopy(OperandAsm dest, OperandAsm source, int numBytes)
{
	int num16ByteCopies = numBytes / 16;
	bool extra8ByteCopy = numBytes % 16 != 0;

	for (int i = 0; i < num16ByteCopies; ++i)
	{
		
		REGISTER reg = AllocateTempRegister(source, true, true, true);
		dest.xmmwordSize = true;
		StatementAsm movStatement(x64_MOVUPS, dest, OperandAsm::CreateRegisterOperand(reg));
		statements.push_back(std::move(movStatement));

		dest.baseOffset += 16;
		source.baseOffset += 16;
	}

	if (extra8ByteCopy)
	{
		REGISTER reg = AllocateTempRegister(source, true, true, false);
		dest.xmmwordSize = false;
		StatementAsm movStatement(x64_MOVS, dest, OperandAsm::CreateRegisterOperand(reg));
		statements.push_back(std::move(movStatement));
	}
	
}
