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


