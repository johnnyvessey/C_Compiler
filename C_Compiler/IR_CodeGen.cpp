#include "IR_CodeGen.h"
#include <unordered_set>

using std::unordered_set;

void IR_CodeGen::ConvertToIR()
{
	statements->ConvertStatementToIR(irState);
}

IR_CodeGen::IR_CodeGen(shared_ptr<StatementGroup> statements) : statements(statements) {}

void IR_CodeGen::PrintIR()
{
	//print struct definitions
	//print function definitions

	//print regular statements

	std::cout << ".data\n";
	for (int i = 0; i < irState.floatLiteralGlobals.size(); ++i)
	{
		std::cout << "\tvar" << (i + 1) << " dq " << irState.floatLiteralGlobals.at(i) << "\n";
	}

	std::cout << "\n\n.code\n\n";

	for (const IR_Function_Group& function : irState.functions)
	{
		for (const shared_ptr<IR_Statement>& statement : function.IR_statements)
		{
			std::cout << statement->ToString() << "\n";
		}
		std::cout << "-----------\n";
	}

}


void IR_CodeGen::Optimize()
{
	for (IR_Function_Group& function : irState.functions)
	{
		optimizationSettings.OptimizeJumps(function.IR_statements);
	}
}


void IR_Optimizations::RemoveNoOps(vector<shared_ptr<IR_Statement>>& irStatements)
{
	vector<shared_ptr<IR_Statement>> newStatements;
	newStatements.reserve(irStatements.size());

	for (const shared_ptr<IR_Statement>& statement : irStatements)
	{
		if (statement->GetType() != _IR_NOP)
		{
			newStatements.push_back(statement);
		}
	}
	newStatements.shrink_to_fit();
	irStatements = newStatements;
}


void IR_Optimizations::OptimizeJumps(vector<shared_ptr<IR_Statement>>& irStatements)
{
	if (optimizeJumps)
	{
		RemoveJumpToNextLine(irStatements);
		RemoveConsecutiveJumps(irStatements);
		RemoveUnusedLabels(irStatements);
	}
}

/*
	Remove a jump to a label that is on the next line
	i.e.:

	JLTE #1
	Label #1

	This is useful for simplifying the if/then chains and and/or conditional parsing.
*/
void IR_Optimizations::RemoveJumpToNextLine(vector<shared_ptr<IR_Statement>>& irStatements)
{

	for (int i = 0; i < int(irStatements.size()) - 1; ++i)
	{
		const shared_ptr<IR_Statement>& statement = irStatements.at(i);
		const shared_ptr<IR_Statement>& nextStatement = irStatements.at(i + 1);

		if (statement->GetType() == _IR_JUMP && nextStatement->GetType() == _IR_LABEL)
		{
			IR_Jump* jump = dynamic_cast<IR_Jump*>(statement.get());
			IR_Label* label = dynamic_cast<IR_Label*>(nextStatement.get());

			if (jump->labelIdx == label->label)
			{
				irStatements[i] = make_shared<IR_NOP>();
			}
		}
	}

	RemoveNoOps(irStatements);
}
void IR_Optimizations::RemoveConsecutiveJumps(vector<shared_ptr<IR_Statement>>& irStatements)
{


	RemoveNoOps(irStatements);
}

/*
	Remove labels that are never jumped to.
*/
void IR_Optimizations::RemoveUnusedLabels(vector<shared_ptr<IR_Statement>>& irStatements)
{

	unordered_set<int> labelSet;

	for (const shared_ptr<IR_Statement>& statement : irStatements)
	{
		if (statement->GetType() == _IR_JUMP)
		{
			IR_Jump* jump = dynamic_cast<IR_Jump*>(statement.get());

			labelSet.insert(jump->labelIdx);
		}
	}

	for (shared_ptr<IR_Statement>& statement : irStatements)
	{
		if (statement->GetType() == _IR_LABEL)
		{
			IR_Label* irLabel = dynamic_cast<IR_Label*>(statement.get());

			if (labelSet.find(irLabel->label) == labelSet.end())
			{
				statement = make_shared<IR_NOP>();
			}
		}
	}

	RemoveNoOps(irStatements);
}


void IR_CodeGen::AllocateRegisters()
{
	RegisterAllocator registerAllocator;
	//registerAllocator.AllocateRegisters(irState.functions);
}