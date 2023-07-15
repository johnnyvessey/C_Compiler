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

StatementType StatementGroup::GetStatementType()
{
	return _STATEMENT_GROUP;
}

void StatementGroup::ConvertStatementToIR(IR& irState)
{
	vector<IR_Statement> ir_statements;
	for (const unique_ptr<Statement>& statement : statements)
	{
		statement->ConvertStatementToIR(irState);	
	}

}

void AST_Initialization::PrintStatementAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Initialize: \n";// << lvalue->name << "; type: " << lvalue->type.lValueType << " ;ptr: " << lvalue->type.pointerLevel << " " << lvalue->type.structName << "\n";
	std::cout << string(indentLevel + 1, '\t') << "Name: " << lvalue->name << "; type: " << lvalue->type.lValueType << " " 
		<< lvalue->type.structName << " " << string(lvalue->type.pointerLevel, '*') << "\n";
	if (rvalue) {
		rvalue->PrintExpressionAST(indentLevel + 1);
	}

}

StatementType AST_Initialization::GetStatementType()
{
	return _INITIALIZATION;
}

void AST_Initialization::ConvertStatementToIR(IR& irState)
{
}



void AST_Assignment::PrintStatementAST(int indentLevel)
{
	expr->PrintExpressionAST(indentLevel);
}

void AST_Assignment::ConvertStatementToIR(IR& irState)
{

}

StatementType AST_Assignment::GetStatementType()
{
	return _ASSIGNMENT;
}




AST_Else_If::AST_Else_If()
{
	statements = make_unique<StatementGroup>();
}

void AST_Else_If::ConvertStatementToIR(IR& irState)
{
	//TODO: DEFINE THIS
}

StatementType AST_Else_If::GetStatementType()
{
	return _ELSE_IF;
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

void AST_If_Then::ConvertStatementToIR(IR& irState)
{
	//TODO: DEFINE THIS
}

StatementType AST_If_Then::GetStatementType()
{
	return _IF_THEN;
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
	std::cout << string(indentLevel, '\t') << "Function definition: name = " << func->def.name << "; type = " << func->def.returnType.lValueType
		<< " " << func->def.returnType.structName << " " << string(func->def.returnType.pointerLevel, '*') << "\n";
	std::cout << string(indentLevel + 1, '\t') << "Args: ";
	for (const Variable& arg : func->def.arguments)
	{
		std::cout << arg.type.lValueType << " " << arg.type.structName << " " << string(arg.type.pointerLevel, '*') << " " << arg.name << ", ";
	}
	std::cout << "\n";

	func->statements->PrintStatementAST(indentLevel + 1);
}

void AST_Function_Definition::ConvertStatementToIR(IR& irState)
{
	//TODO: DEFINE THIS
}

StatementType AST_Function_Definition::GetStatementType()
{
	return _FUNCTION_DEFINITION;
}



AST_Struct_Definition::AST_Struct_Definition(string name, unordered_map<string, Struct_Variable>&& variables, vector<Struct_Variable>&& structVariables, size_t memorySize)
	
{
		//figure out if I need to align struct (to 4 bytes, for example)

		def.name = name;
		def.memorySize = memorySize;
		def.variableMapping = variables; //SHOULD WE USE STD::MOVE???
		def.variableVector = structVariables; 
		
}

void AST_Struct_Definition::ConvertStatementToIR(IR& irState)
{
	//TODO: DEFINE THIS
}

StatementType AST_Struct_Definition::GetStatementType()
{
	return _STRUCT_DEFINITION;
}


AST_Struct_Definition::AST_Struct_Definition() {
	//memorySize = 0;
}


void AST_Struct_Definition::PrintStatementAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Struct definition: " << def.name << " (size: " << def.memorySize << ")" << ":\n";
	for (const Struct_Variable& structVar : def.variableVector) //may be out of order
	{
		std::cout << string(indentLevel + 1, '\t') << structVar.v.name << ": " << structVar.v.type.lValueType << " " << structVar.v.type.structName
			<< string(structVar.v.type.pointerLevel, '*') << " (offset: " << structVar.memoryOffset << ")\n";
	}
}


void AST_Expression_Statement::PrintStatementAST(int indentLevel)
{
	expr->PrintExpressionAST(indentLevel);
}

void AST_Expression_Statement::ConvertStatementToIR(IR& irState)
{
	//TODO: DEFINE THIS
}

StatementType AST_Expression_Statement::GetStatementType()
{
	return _EXPRESSION;
}


void AST_Return_Statement::PrintStatementAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "return: \n";
	if (returnExpression)
	{
		returnExpression->PrintExpressionAST(indentLevel + 1);
	}
 }

void AST_Return_Statement::ConvertStatementToIR(IR& irState)
{
	//TODO: DEFINE THIS
}

StatementType AST_Return_Statement::GetStatementType()
{
	return _RETURN;
}

StatementType AST_NOP::GetStatementType()
{
	return _NOP;
}

void AST_NOP::ConvertStatementToIR(IR& irState)
{

}

void AST_NOP::PrintStatementAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "NOP\n";
}

StatementType AST_For_Loop::GetStatementType()
{
	return _FOR_LOOP;
}

void AST_For_Loop::ConvertStatementToIR(IR& irState)
{
	//TODO: DEFINE THIS
}

void AST_For_Loop::PrintStatementAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "For:\n";
	First->PrintStatementAST(indentLevel + 1);
	Condition->PrintExpressionAST(indentLevel + 1);
	Third->PrintExpressionAST(indentLevel + 1);
	std::cout << string(indentLevel, '\t') << "------\n";
	Statements->PrintStatementAST(indentLevel + 1);
}

StatementType AST_While_Loop::GetStatementType()
{
	return _WHILE_LOOP;
}

void AST_While_Loop::ConvertStatementToIR(IR& irState)
{
	//TODO: DEFINE THIS
}

void AST_While_Loop::PrintStatementAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "While:\n";
	Condition->PrintExpressionAST(indentLevel + 1);
	std::cout << string(indentLevel, '\t') << "------\n";
	Statements->PrintStatementAST(indentLevel + 1);
}


StatementType AST_Continue::GetStatementType()
{
	return _CONTINUE;
}

void AST_Continue::ConvertStatementToIR(IR& irState)
{
	//TODO: DEFINE THIS
}
void AST_Continue::PrintStatementAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Continue Loop\n";
}

StatementType AST_Break::GetStatementType()
{
	return _BREAK;
}

void AST_Break::ConvertStatementToIR(IR& irState)
{
	//TODO: DEFINE THIS
}

void AST_Break::PrintStatementAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Break Loop\n";
}