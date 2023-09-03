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
		"rax",
		"rbx",
		"rcx",
		"rdx",
		"rsi",
		"rdi",
		"rbp",
		"rsp",
		"r8",
		"r9",
		"r10",
		"r11",
		"r12",
		"r13",
		"r14",
		"r15",
		"xmm0",
		"xmm1",
		"xmm2",
		"xmm3",
		"xmm4",
		"xmm5",
		"xmm6",
		"xmm7",
		"xmm8",
		"xmm9",
		"xmm10",
		"xmm11",
		"xmm12",
		"xmm13",
		"xmm14",
		"xmm15"
};

const vector<string> RegisterString::registerStringMapping1byte = {
		"al",
		"bl",
		"cl",
		"dl",
		"sil",
		"dil",
		"bpl",
		"spl",
		"r8b",
		"r9b",
		"r10b",
		"r11b",
		"r12b",
		"r13b",
		"r14b",
		"r15b"
};

LoopLabel::LoopLabel(int labelIdx, int labelStatementIdx) : labelIdx(labelIdx), labelStatementIdx(labelStatementIdx) {}


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
	return GetPointerLevel() > 0 ? POINTER_SIZE : REGISTER_SIZE;
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
