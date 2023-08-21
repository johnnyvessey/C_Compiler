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

		irState.add_statement(make_shared<IR_ContinuousMemoryInit>(structInit));
		IR_Value structVal(IR_STRUCT, IR_VARIABLE, structInit.byteNum, structInit.varIdx, false, "", IR_NONE);
		irState.state.scope.variableMapping.back()[lvalue->name] = structVal;

		if (rvalue)
		{
			IR_Operand irRValue = rvalue->ConvertExpressionToIR(irState);
			irState.add_statement(make_shared<IR_Assign>(IR_Assign(irRValue.value.type, IR_STRUCT_COPY, structInit.byteNum,IR_Operand(structVal), irRValue)));
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

		irState.add_statement(make_shared<IR_ContinuousMemoryInit>(arrayInit));

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
		
		//TODO: check if we need init statement
		//irState.add_statement(make_shared<IR_VariableInit>(IR_VariableInit(value)));

		if (rvalue)
		{
			//add assign statement afterwards
			IR_Operand irRValue = rvalue->ConvertExpressionToIR(irState);
			if (irRValue.value.specialVars == IR_FLAGS)
			{
				irState.add_statement(make_shared<IR_Assign>(IR_Assign(irRValue.value.type, IR_FLAG_CONVERT, 4, IR_Operand(value), IR_Operand(irRValue))));
			}
			else {
				IR_Assign assign = IR_Assign(irRValue.value.type, IR_COPY, value.byteSize, IR_Operand(value), IR_Operand(irRValue));

				if (rvalue->GetExpressionType() == _Pointer_Offset)
				{
					assign.assignType = IR_LEA;
					assign.source.value.byteSize = POINTER_SIZE;
					assign.source.dereference = true;
				}

				if (assign.source.dereference && assign.source.useMemoryAddress)
				{
					assign.source.useMemoryAddress = false;
					assign.assignType = IR_LEA;
				}
				irState.add_statement(make_shared<IR_Assign>(assign));
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
	//already parsed in AST_If_Then statement
	return;
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
		irState.add_statement(make_shared<IR_Jump>(IR_Jump(falseLabel, (FlagResults) -irCondition.value.flag)));
		irState.add_statement(make_shared<IR_Jump>(IR_Jump(trueLabel, IR_ALWAYS)));

	}

}


void AST_If_Then::ConvertStatementToIR(IR& irState)
{
	int trueLabelIdx;
	int falseLabelIdx;
	int endLabelIdx = irState.state.labelIndex++;

	ParseIfThenCondition(condition.get(), irState, trueLabelIdx, falseLabelIdx);
	irState.add_statement(make_shared<IR_Label>(IR_Label(trueLabelIdx)));
	irState.EnterScope();
	ifStatement->ConvertStatementToIR(irState); //convert statements inside if to IR
	irState.ExitScope();

	if (!elseIfStatements.empty() || !elseStatement->statements.empty())
	{
		irState.add_statement(make_shared<IR_Jump>(IR_Jump(endLabelIdx, IR_ALWAYS)));
	}

	int currentFalseLabel = falseLabelIdx;
	for (const unique_ptr<AST_Else_If>& elseIf : elseIfStatements)
	{ 
		irState.add_statement(make_shared<IR_Label>(IR_Label(currentFalseLabel)));

		
		int trueLabelElseIf;
		int falseLabelElseIf;
		ParseIfThenCondition(elseIf->condition.get(), irState, trueLabelElseIf, falseLabelElseIf);

		irState.add_statement(make_shared<IR_Label>(IR_Label(trueLabelElseIf)));

		irState.EnterScope();
		elseIf->statements->ConvertStatementToIR(irState);
		irState.ExitScope();

		currentFalseLabel = falseLabelElseIf;
		irState.add_statement(make_shared<IR_Jump>(IR_Jump(endLabelIdx, IR_ALWAYS)));
	}

	irState.add_statement(make_shared<IR_Label>(IR_Label(currentFalseLabel)));

	if (!this->elseStatement->statements.empty())
	{
		irState.EnterScope();
		elseStatement->ConvertStatementToIR(irState);
		irState.ExitScope();

		irState.add_statement(make_shared<IR_Jump>(IR_Jump(endLabelIdx, IR_ALWAYS)));
	}

	irState.add_statement(make_shared<IR_Label>(IR_Label(endLabelIdx)));

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
	string functionName = this->func->def.name;
	irState.state.scope.currentFunction = functionName;
	irState.functions.push_back(IR_Function_Group(functionName));

	shared_ptr<IR_FunctionLabel> funcLabel = make_shared<IR_FunctionLabel>(IR_FunctionLabel(functionName));

	irState.add_statement(funcLabel);
	irState.EnterFunction();

	Variable returnVar;
	returnVar.type = this->func->def.returnType;
	IR_Value retType = Expression::VariableToIR_Value(returnVar, irState);

	IR_Value retValue;

	if (retType.pointerLevel > 0)
	{
		retValue = irState.state.functionReturnValueInt;
		retValue.byteSize = POINTER_SIZE;
		funcLabel->returnValueByteSize = POINTER_SIZE;
	}
	else if (retType.type != IR_STRUCT)
	{
		retValue = retType.type == IR_INT ? irState.state.functionReturnValueInt : irState.state.functionReturnValueFloat;
		retValue.byteSize = 4; //TODO: change this to be based on memory size of variable (not IR variable, because IR_INT can have multiple byte sizes)
		funcLabel->returnValueByteSize = 4;
	}
	else
	{
		retValue = irState.state.functionReturnValueStructPointer;
		retValue.byteSize = POINTER_SIZE;
		StructDefinition structDef = irState.state.scope.FindStruct(this->func->def.returnType.structName);
		funcLabel->returnValueByteSize = GetMemorySizeForIR(this->func->def.returnType, &structDef);
	}

	funcLabel->returnValue = retValue;

	//set up function arguments -> add them to scope dictionary
	for (const Variable& v : this->func->def.arguments)
	{
		IR_Value value = Expression::VariableToIR_Value(v, irState);
		funcLabel->args.push_back(value);

		//add variables to scope of function
		irState.state.scope.variableMapping.back()[v.name] = value;
	}

	irState.state.scope.functionMapping[this->func->def.name] = *funcLabel;
	int endFuncLabel = irState.state.labelIndex++;
	irState.state.scope.functionEndLabel = endFuncLabel;
	

	this->func->statements->ConvertStatementToIR(irState);

	irState.add_statement(make_shared<IR_Label>(IR_Label(endFuncLabel)));

	irState.ExitFunction();

	irState.add_statement(make_shared<IR_Return>(IR_Return()));
	irState.state.scope.functionEndLabel = 0; //probably unnecessary
	irState.state.scope.currentFunction = "";
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
	if (this->returnExpression)
	{
		IR_Operand returnExpression = this->returnExpression->ConvertExpressionToIR(irState);

		if (returnExpression.value.type == IR_INT)
		{
			IR_Value returnRegister = irState.state.functionReturnValueInt;
			returnRegister.byteSize = returnExpression.GetByteSize();
			returnRegister.pointerLevel = returnExpression.GetPointerLevel();
			irState.add_statement(make_shared<IR_Assign>(IR_Assign(IR_INT, IR_COPY, returnExpression.value.byteSize,
				IR_Operand(returnRegister), returnExpression)));
		}
		else if (returnExpression.value.type == IR_FLOAT)
		{
			IR_Value returnRegister = irState.state.functionReturnValueFloat;
			returnRegister.byteSize = returnExpression.GetByteSize();
			returnRegister.pointerLevel = returnExpression.GetPointerLevel();

			irState.add_statement(make_shared<IR_Assign>(IR_Assign(IR_FLOAT, IR_COPY, returnExpression.value.byteSize,
				IR_Operand(returnRegister), returnExpression)));
		}
		else {
			//returning struct

			//first argument passed in is pointer to struct
			IR_Value returnRegister = irState.state.functionReturnValueStructPointer;
			returnRegister.byteSize = POINTER_SIZE;
			returnRegister.pointerLevel = returnExpression.GetPointerLevel();
			IR_Operand returnOp(returnRegister);
			returnOp.dereference = true;
			//TODO: figure out if first operand needs to be dereferenced
			irState.add_statement(make_shared<IR_Assign>(IR_Assign(IR_STRUCT, IR_STRUCT_COPY, returnExpression.value.byteSize,
				returnOp, returnExpression)));

		}
	}
	
	irState.add_statement(make_shared<IR_Jump>(IR_Jump(irState.state.scope.functionEndLabel, IR_ALWAYS)));

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
	return; //do nothing
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
	int startLabelIdx = irState.state.labelIndex++;
	int endLabelIdx = 0;
	int postConditionLabelIdx = 0;

	//set up start and end labels for break and continue to reference (save previous start and end labels, and restore them at the end)
	int prevStartLabelIdx = irState.state.scope.currentLoopStartLabelIdx;
	int prevEndLabelIdx = irState.state.scope.currentLoopEndLabelIdx;

	//start of loop
	irState.add_statement(make_shared<IR_LoopStart>(IR_LoopStart()));
	irState.EnterScope();

	this->First->ConvertStatementToIR(irState);

	int loopBeginIdx = irState.state.labelIndex++;
	irState.add_statement(make_shared<IR_Label>(IR_Label(loopBeginIdx)));

	ParseIfThenCondition(this->Condition.get(), irState, postConditionLabelIdx, endLabelIdx);

	irState.state.scope.currentLoopStartLabelIdx = startLabelIdx;
	irState.state.scope.currentLoopEndLabelIdx = endLabelIdx;

	//loop body
	irState.add_statement(make_shared<IR_Label>(IR_Label(postConditionLabelIdx)));

	this->Statements->ConvertStatementToIR(irState);

	irState.add_statement(make_shared<IR_Label>(IR_Label(startLabelIdx)));
	this->Third->ConvertExpressionToIR(irState);

	irState.add_statement(make_shared<IR_Jump>(IR_Jump(loopBeginIdx, IR_ALWAYS)));

	//end of loop
	irState.add_statement(make_shared<IR_Label>(IR_Label(endLabelIdx)));
	irState.ExitScope();
	irState.add_statement(make_shared<IR_LoopEnd>(IR_LoopEnd()));

	//reset loop label indices to previous values
	irState.state.scope.currentLoopStartLabelIdx = prevStartLabelIdx;
	irState.state.scope.currentLoopEndLabelIdx = prevEndLabelIdx;

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
	int startLabelIdx = irState.state.labelIndex++;
	int endLabelIdx = 0;
	int postConditionLabelIdx = 0;

	//set up start and end labels for break and continue to reference (save previous start and end labels, and restore them at the end)
	int prevStartLabelIdx = irState.state.scope.currentLoopStartLabelIdx;
	int prevEndLabelIdx = irState.state.scope.currentLoopEndLabelIdx;


	//start of loop
	irState.EnterScope();

	irState.add_statement(make_shared<IR_Label>(IR_Label(startLabelIdx)));

	ParseIfThenCondition(this->Condition.get(), irState, postConditionLabelIdx, endLabelIdx);

	irState.state.scope.currentLoopStartLabelIdx = startLabelIdx;
	irState.state.scope.currentLoopEndLabelIdx = endLabelIdx;

	//loop body
	irState.add_statement(make_shared<IR_LoopStart>(IR_LoopStart()));
	irState.add_statement(make_shared<IR_Label>(IR_Label(postConditionLabelIdx)));
	
	this->Statements->ConvertStatementToIR(irState);
	irState.add_statement(make_shared<IR_Jump>(IR_Jump(startLabelIdx, IR_ALWAYS)));

	//end of loop
	irState.add_statement(make_shared<IR_Label>(IR_Label(endLabelIdx)));
	irState.add_statement(make_shared<IR_LoopEnd>(IR_LoopEnd()));

	irState.ExitScope();

	irState.state.scope.currentLoopStartLabelIdx = prevStartLabelIdx;
	irState.state.scope.currentLoopEndLabelIdx = prevEndLabelIdx;
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
	if (irState.state.scope.currentLoopStartLabelIdx == 0)
	{
		std::cout << "Can't use continue outside of a loop\n";
		throw 0;
	}
	irState.add_statement(make_shared<IR_Jump>(IR_Jump(irState.state.scope.currentLoopStartLabelIdx, IR_ALWAYS)));
}
void AST_Continue::PrintStatementAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Continue Loop\n";
}

StatementType AST_Break::GetStatementType()
{
	return _BREAK;
}


//TODO: figure out if I need to add SCOPE_END right before break statement???
void AST_Break::ConvertStatementToIR(IR& irState)
{
	if (irState.state.scope.currentLoopEndLabelIdx == 0)
	{
		std::cout << "Can't use break outside of a loop\n";
		throw 0;
	}

	irState.add_statement(make_shared<IR_Jump>(IR_Jump(irState.state.scope.currentLoopEndLabelIdx, IR_ALWAYS)));
}

void AST_Break::PrintStatementAST(int indentLevel)
{
	std::cout << string(indentLevel, '\t') << "Break Loop\n";
}