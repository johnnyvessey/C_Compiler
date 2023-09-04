#pragma once
#include "x64_State.h"
#include "IR.h"

using std::stringstream;
using std::shared_ptr;

struct x64_CodeGen
{
	IR irState;
	x64_State state;

	void GenerateCode();
	string CodeToString();

	x64_CodeGen(IR& irState);

	void PrintNonRegisterIRVariables();
	void PrintVariableRanges();
	void PrintCurrentRegisterMapping();

	void GenerateDataSection(IR& irState);
};