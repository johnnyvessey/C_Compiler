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
	std::cout << string(indentLevel + 1, '\t') << "Name: " << lvalue->name << "; type: " << lvalue->type.lValueType;
	if (lvalue->arraySize > 0)
	{
		std::cout << "[" << lvalue->arraySize << "]";
	}
	std::cout << " " << lvalue->type.structName << " " << string(lvalue->type.pointerLevel, '*') << "\n";
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
	if (lvalue->type.lValueType == LValueType::STRUCT && lvalue->type.pointerLevel == 0)
	{
		//do struct init here
		StructDefinition structDef = irState.state.scope.FindStruct(lvalue->type.structName);
		IR_ContinuousMemoryInit structInit;
		structInit.byteNum = (int)structDef.memorySize;
		structInit.varIdx = irState.state.varIndex++;

		irState.IR_statements.push_back(make_shared<IR_ContinuousMemoryInit>(structInit));
		IR_Value structVal(IR_STRUCT, IR_VARIABLE, structInit.byteNum, structInit.varIdx, false, "", IR_NONE);
		irState.state.scope.variableMapping.back()[lvalue->name] = structVal;

		if (rvalue)
		{
			IR_Operand irRValue = rvalue->ConvertExpressionToIR(irState);
			irState.IR_statements.push_back(make_unique<IR_Assign>(IR_Assign(irRValue.value.type, IR_STRUCT_COPY, structInit.byteNum,IR_Operand(structVal), irRValue)));
		}
	}
	else if (lvalue->arraySize > 0)
	{
		IR_ContinuousMemoryInit arrayInit;

		VariableType varType = lvalue->type;
		--varType.pointerLevel;

		StructDefinition structDef;
		
		if (lvalue->type.lValueType == LValueType::STRUCT)
		{
			structDef = irState.state.scope.FindStruct(lvalue->type.structName);
		}
		arrayInit.byteNum = lvalue->arraySize * GetMemorySizeForIR(varType, &structDef);
		arrayInit.varIdx = irState.state.varIndex++;

		irState.IR_statements.push_back(make_shared<IR_ContinuousMemoryInit>(arrayInit));

		IR_VarType baseType;
		if (lvalue->type.lValueType == LValueType::STRUCT)
		{
			baseType = IR_STRUCT;
		}
		else if (lvalue->type.lValueType == LValueType::FLOAT)
		{
			baseType = IR_FLOAT;
		}
		else {
			baseType = IR_INT;
		}
		IR_Value arrayVal(IR_INT, IR_VARIABLE, POINTER_SIZE, arrayInit.varIdx, false, "", IR_NONE, lvalue->type.pointerLevel, baseType);

		irState.state.scope.variableMapping.back()[lvalue->name] = arrayVal;

	}
	else
	{
		IR_Value value;
		value.varIndex = irState.state.varIndex++;
		value.valueType = IR_VARIABLE;
		value.type = (lvalue->type.pointerLevel > 0 || lvalue->type.lValueType == LValueType::INT) ? IR_INT : IR_FLOAT;
		//value.varIndex = irState.state.varIndex++; //decide how to do this when assigning to expression to prevent lots of duplicate copying
		value.byteSize = (lvalue->type.pointerLevel > 0) ? POINTER_SIZE : 4;
		value.pointerLevel = lvalue->type.pointerLevel;
		value.baseType = lvalue->type.lValueType ==  LValueType::INT ? IR_INT : IR_FLOAT; //TODO: Figure out other types
		value.isTempValue = false;
		irState.state.scope.variableMapping.back()[lvalue->name] = value; //add to variable dictionary
		
		irState.IR_statements.push_back(make_unique<IR_VariableInit>(IR_VariableInit(value)));

		if (rvalue)
		{
			//add assign statement afterwards
			IR_Operand irRValue = rvalue->ConvertExpressionToIR(irState);
			if (irRValue.value.specialVars == IR_FLAGS)
			{
				irState.IR_statements.push_back(make_unique<IR_Assign>(IR_Assign(irRValue.value.type, IR_FLAG_CONVERT, 4, IR_Operand(value), IR_Operand(irRValue))));
			}
			else {
				irState.IR_statements.push_back(make_unique<IR_Assign>(IR_Assign(irRValue.value.type, IR_COPY, value.byteSize, IR_Operand(value), IR_Operand(irRValue))));
			}
		}

	}



}



void AST_Assignment::PrintStatementAST(int indentLevel)
{
	expr->PrintExpressionAST(indentLevel);
}

void AST_Assignment::ConvertStatementToIR(IR& irState)
{
	this->expr->ConvertExpressionToIR(irState);
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

void ParseIfThenCondition(Expression* condition, IR& irState, int& trueLabel, int& falseLabel)
{
	IR_Operand irCondition = Expression::ParseBooleanExpression(condition, irState, false, trueLabel, falseLabel, false);

	if (!Expression::isAndOrExpression(condition))
	{
		irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(falseLabel, (IR_FlagResults) -irCondition.value.flag)));
		irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(trueLabel, IR_ALWAYS)));

	}

}


void AST_If_Then::ConvertStatementToIR(IR& irState)
{
	int trueLabelIdx;
	int falseLabelIdx;
	int endLabelIdx = irState.state.labelIndex++;

	ParseIfThenCondition(condition.get(), irState, trueLabelIdx, falseLabelIdx);
	irState.IR_statements.push_back(make_shared<IR_Label>(IR_Label(trueLabelIdx)));
	irState.EnterScope();
	ifStatement->ConvertStatementToIR(irState); //convert statements inside if to IR
	irState.ExitScope();
	irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(endLabelIdx, IR_ALWAYS)));

	int currentFalseLabel = falseLabelIdx;
	for (const unique_ptr<AST_Else_If>& elseIf : elseIfStatements)
	{ 
		irState.IR_statements.push_back(make_shared<IR_Label>(IR_Label(currentFalseLabel)));

		
		int trueLabelElseIf;
		int falseLabelElseIf;
		ParseIfThenCondition(elseIf->condition.get(), irState, trueLabelElseIf, falseLabelElseIf);

		irState.IR_statements.push_back(make_shared<IR_Label>(IR_Label(trueLabelElseIf)));

		irState.EnterScope();
		elseIf->statements->ConvertStatementToIR(irState);
		irState.ExitScope();

		currentFalseLabel = falseLabelElseIf;
		irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(endLabelIdx, IR_ALWAYS)));
	}
	if (this->elseStatement)
	{
		irState.IR_statements.push_back(make_shared<IR_Label>(IR_Label(currentFalseLabel)));

		irState.EnterScope();
		elseStatement->ConvertStatementToIR(irState);
		irState.ExitScope();

		irState.IR_statements.push_back(make_shared<IR_Jump>(IR_Jump(endLabelIdx, IR_ALWAYS)));

	}

	irState.IR_statements.push_back(make_shared<IR_Label>(IR_Label(endLabelIdx)));

}

StatementType AST_If_Then::GetStatementType()
{
	return _IF_THEN;
}



void AST_If_Then::PrintStatementAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "if statement:\n";
	condition->PrintExpressionAST(indentLevel);
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



AST_Struct_Definition::AST_Struct_Definition(string name, unordered_map<string, Struct_Variable>&& variables, vector<Struct_Variable>&& structVariables, int memorySize)
	
{
		//figure out if I need to align struct (to 4 bytes, for example)

		def.name = name;
		def.memorySize = memorySize;
		def.variableMapping = variables; //SHOULD WE USE STD::MOVE???
		def.variableVector = structVariables; 
		
}

void AST_Struct_Definition::ConvertStatementToIR(IR& irState)
{
	irState.state.scope.structMapping.back()[def.name] = def;
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
			<< string(structVar.v.type.pointerLevel, '*');
		
		if (structVar.v.arraySize > 0)
		{
			std::cout << "[" << structVar.v.arraySize << "]";
		}
		std::cout << " (offset: " << structVar.memoryOffset << ")\n";
	}
}


void AST_Expression_Statement::PrintStatementAST(int indentLevel)
{
	expr->PrintExpressionAST(indentLevel);
}

void AST_Expression_Statement::ConvertStatementToIR(IR& irState)
{
	this->expr->ConvertExpressionToIR(irState);
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