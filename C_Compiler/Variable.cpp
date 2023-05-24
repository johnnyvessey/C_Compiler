#include "Variable.h"

using namespace VariableNamespace;

VariableType::VariableType()
{
	pointerLevel = 0;
}

bool VariableType::operator==(const VariableType& rhs)
{
	return this->lValueType == rhs.lValueType && this->structName == rhs.structName && this->pointerLevel == rhs.pointerLevel;
}
