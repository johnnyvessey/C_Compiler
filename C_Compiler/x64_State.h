#pragma once

#include "RegisterAllocator.h"
#include "x64_Statement.h"
#include <unordered_set>
#include <algorithm>
#include <unordered_map>

using std::unordered_set;
using std::unordered_map;
using std::pair;

struct x64_State
{
	int lineNum = 0;
	RegisterAllocator registerAllocator;
	IR_VariableData irVariableData;
	vector<StatementAsm> statements;

	const static vector<REGISTER> _calleeSavedRegisters;
	const static vector<REGISTER> _usableIntCalleeSavedRegisters;
	const static vector<REGISTER> _usableFloatCalleeSavedRegisters;

	inline static bool isIntRegister(REGISTER reg)
	{
		return (int)reg < (int)XMM0;
	}
	inline static bool isCalleeSavedRegister(REGISTER reg)
	{
		return std::find(_calleeSavedRegisters.begin(), _calleeSavedRegisters.end(), reg) != _calleeSavedRegisters.end();
	}

	OperandAsm AllocateRegister(IR_Value value, REGISTER specificRegister = _NONE);

	void ExpireOldIntervals();
};
