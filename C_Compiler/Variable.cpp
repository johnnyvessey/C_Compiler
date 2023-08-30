#include "Variable.h"

using namespace VariableNamespace;

VariableType::VariableType()
{
	pointerLevel = 0;
}

VariableType::VariableType(LValueType type, string structName, int pointerLevel) : lValueType(type), structName(structName), pointerLevel(pointerLevel) {}


bool VariableType::operator==(const VariableType& rhs)
{
	return this->lValueType == rhs.lValueType && this->structName == rhs.structName && this->pointerLevel == rhs.pointerLevel;
}

memoryOffset::memoryOffset(int offset, bool isRsp) : offset(offset), isRsp(isRsp) {}
memoryOffset::memoryOffset() {}

const vector<string> RegisterString::registerStringMapping = {
		"RAX",
		"RBX",
		"RCX",
		"RDX",
		"RSI",
		"RDI",
		"RBP",
		"RSP",
		"R8",
		"R9",
		"R10",
		"R11",
		"R12",
		"R13",
		"R14",
		"R15",
		"XMM0",
		"XMM1",
		"XMM2",
		"XMM3",
		"XMM4",
		"XMM5",
		"XMM6",
		"XMM7",
		"XMM8",
		"XMM9",
		"XMM10",
		"XMM11",
		"XMM12",
		"XMM13",
		"XMM14",
		"XMM15"
};

IR_Value::IR_Value() {}
IR_Value::IR_Value(IR_VarType type, IR_ValueType valueType, int byteSize, int varIndex, bool isTempValue, string literalValue, IR_SpecialVars specialVars) :
	type(type), valueType(valueType), byteSize(byteSize), varIndex(varIndex), isTempValue(isTempValue), literalValue(literalValue), specialVars(specialVars),
	baseType(type) {}

IR_Value::IR_Value(IR_VarType type, IR_ValueType valueType, int byteSize, int varIndex, bool isTempValue, string literalValue, IR_SpecialVars specialVars,
	int pointerLevel, IR_VarType baseType) :
	type(type), valueType(valueType), byteSize(byteSize), varIndex(varIndex), isTempValue(isTempValue), literalValue(literalValue), specialVars(specialVars),
	pointerLevel(pointerLevel), baseType(baseType) {}

IR_Operand::IR_Operand() {}
IR_Operand::IR_Operand(IR_Value value) : value(value), baseOffset(0) {}

int IR_Operand::GetPointerLevel()
{
	int pointerLevelOffset = dereference ? -1 : 0;
	int refOffset = useMemoryAddress ? 1 : 0;

	return value.pointerLevel + pointerLevelOffset + refOffset;
}
IR_VarType IR_Operand::GetVarType()
{
	return GetPointerLevel() > 0 ? IR_VarType::IR_INT : this->value.baseType;
}
int IR_Operand::GetByteSize()
{
	return GetPointerLevel() > 0 ? POINTER_SIZE : 4;
}

RegisterVariableGroup::RegisterVariableGroup() : variableIndex(0), isModified(true) {}
RegisterVariableGroup::RegisterVariableGroup(int varIndex, bool isModified) : variableIndex(varIndex), isModified(isModified) {}

RegisterMapping::RegisterMapping()
{
	regMapping = vector<RegisterVariableGroup>(NUM_REGISTERS, RegisterVariableGroup());
}

void RegisterMapping::SetRegister(int reg, int variable)
{
	RegisterVariableGroup var;
	var.variableIndex = variable;

	regMapping[reg] = var;
}

bool RegisterMapping::FindRegisterOfVariable(int variable, int& reg)
{
	for (int i = 0; i < NUM_REGISTERS; ++i)
	{
		if (regMapping[i].variableIndex == variable)
		{
			reg = i;
			return true;
		}
	}

	return false;
}

JumpRegisterMapping::JumpRegisterMapping(RegisterMapping regMapping, int jumpStatementIndex) : jumpRegMapping(regMapping), jumpStatementIndex(jumpStatementIndex) {}
