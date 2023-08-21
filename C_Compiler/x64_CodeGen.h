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

	inline void PrintNonRegisterIRVariables()
	{
		std::cout << "Non register variables: \n{";
		for (const auto& x : state.irVariableData.nonRegisterVariables)
		{
			std::cout << x << ", ";
		}
		std::cout << "}\n";

	}

	void PrintCurrentRegisterMapping();
};