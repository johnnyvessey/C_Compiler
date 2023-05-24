#include "AST_Statement.h"

using namespace AST_Statement;

void StatementGroup::PrintStatementAST(int indentLevel)
{
	for (const unique_ptr<Statement>& statement : statements)
	{
		statement->PrintStatementAST(indentLevel);
		std::cout << "\n-----------------------\n";
	}
}


void AST_Assignment::PrintStatementAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Assign: " << lvalue->name << "; type: " << lvalue->type.lValueType << " ;ptr: " << lvalue->type.pointerLevel << " " << lvalue->type.structName << "\n";
	if (rvalue) {
		rvalue->PrintExpressionAST(indentLevel + 1);
	}
}



AST_Else_If::AST_Else_If()
{
	statements = make_unique<StatementGroup>();
}

void AST_Else_If::PrintStatementAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "else if: \n";
	condition->PrintExpressionAST(indentLevel);
	statements->PrintStatementAST(indentLevel + 2);
}




AST_If_Then::AST_If_Then()
{
	ifStatement = make_unique<StatementGroup>();
	elseStatement = make_unique<StatementGroup>();
}

void AST_If_Then::PrintStatementAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "if statement:\n";
	Condition->PrintExpressionAST(indentLevel);
	std::cout << string(indentLevel + 1, '\t') << "then:\n";
	ifStatement->PrintStatementAST(indentLevel + 2);

	for (const auto& elseIfStatement : elseIfStatements)
	{
		elseIfStatement->PrintStatementAST(indentLevel + 1);
	}

	if (elseStatement->statements.size() > 0)
	{
		std::cout << string(indentLevel + 1, '\t') << "else:\n";
		elseStatement->PrintStatementAST(indentLevel + 2);
	}
}


//AST_Function_Definition::AST_Function_Definition()
//{
//}
void AST_Function_Definition::PrintStatementAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Function definition: name = " << func->name << "; type = " << func->returnType.lValueType
		<< " " << func->returnType.structName << " " << string(func->returnType.pointerLevel, '*') << "\n";
	std::cout << string(indentLevel + 1, '\t') << "Args: ";
	for (const Variable& arg : func->arguments)
	{
		std::cout << arg.type.lValueType << " " << arg.type.structName << " " << string(arg.type.pointerLevel, '*') << " " << arg.name << ", ";
	}
	std::cout << "\n";

	func->statements->PrintStatementAST(indentLevel + 1);
}




AST_Struct_Definition::AST_Struct_Definition(string name, vector<Struct_Variable>&& variables) : name(name)
{
	size_t currentOffset = 0;
	for (auto&& var : variables)
	{
		var.memoryOffset = currentOffset;
		variableMapping[var.v.name] = var;

		currentOffset += GetMemoryFromType(var.v.type.lValueType, var.v.type.structName);
	}

	memorySize = currentOffset; //figure out if I need to align struct (to 4 bytes, for example)
}

AST_Struct_Definition::AST_Struct_Definition() {}


void AST_Struct_Definition::PrintStatementAST(int indentLevel)
{

}


void AST_Expression_Statement::PrintStatementAST(int indentLevel)
{

}

void AST_Return_Statement::PrintStatementAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "return: \n";
	returnExpression->PrintExpressionAST(indentLevel + 1);
 }
