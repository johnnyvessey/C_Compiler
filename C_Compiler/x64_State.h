#pragma once

#include "RegisterAllocator.h"
#include "x64_Statement.h"
#include <unordered_set>
#include <algorithm>
using std::unordered_set;

struct IR_VariableData
{
	unordered_set<int> nonRegisterVariables;
	//variables ranges also
};
struct x64_State
{
	int lineNum = 0;
	RegisterAllocator registerAllocator;
	IR_VariableData irVariableData;
	vector<StatementAsm> statements;

	const static vector<REGISTER> _calleeSavedRegisters;

	inline static bool isIntRegister(REGISTER reg)
	{
		return (int)reg < (int)XMM0;
	}
	inline static bool isCalleeSavedRegister(REGISTER reg)
	{
		return std::find(_calleeSavedRegisters.begin(), _calleeSavedRegisters.end(), reg) != _calleeSavedRegisters.end();
	}
};