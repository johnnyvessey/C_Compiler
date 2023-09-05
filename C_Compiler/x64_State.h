#pragma once

#include "RegisterAllocator.h"
#include "x64_Statement.h"
#include <unordered_set>
#include <algorithm>
#include <unordered_map>
#include <iostream>

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
	const static vector<REGISTER> _usableIntRegisters;
	const static vector<REGISTER> _usableFloatRegisters;

	const static vector<REGISTER> _functionIntArguments;
	const static vector<REGISTER> _functionFloatArguments;

	inline static bool isIntRegister(REGISTER reg)
	{
		return (int)reg < (int)XMM0;
	}
	inline static bool isCalleeSavedRegister(REGISTER reg)
	{
		return std::find(_calleeSavedRegisters.begin(), _calleeSavedRegisters.end(), reg) != _calleeSavedRegisters.end();
	}

	REGISTER AllocateRegister(IR_Value value, REGISTER specificRegister = _NONE);

	void EvictExpiredVariables();

	REGISTER FindFurthestAwayRegisterInMappingUsed();

	void CreateStackSpaceForVariables(const string& funcName);

	StatementAsm SpillRegister(REGISTER reg, RegisterMapping& mapping);

	void SetUpFunctionVariableMappings(const string& functionName);

	void SetRegisterMappingToIntersectionOfMappings(int labelIdx);

	void MatchRegisterMappingsToIntialMapping(int labelIdx, int labelStatementIdx);

	void SpillRegisterIfChanged(RegisterMapping& mapping, int reg, int jumpStatementIdx = -1);

	REGISTER FindOpenRegister(OperandAsm operand, bool isFloat, bool shouldLoadRegister, bool xmmwordSize);

	REGISTER AllocateTempRegister(OperandAsm operand, bool isFloat, bool shouldLoadRegister = true, bool xmmwordSize = false);

	void StructCopy(OperandAsm dest, OperandAsm source, int numBytes);
};
