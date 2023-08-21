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


const vector<string> REGISTER_MAPPING = {
		"RAX",
		"RBX",
		"RCX",
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