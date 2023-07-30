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
	if (lvalue->type.lValueType == LValueType::STRUCT && lvalue->type.pointerLevel == 0)
	{
		//do struct init here
		StructDefinition structDef = irState.state.scope.FindStruct(lvalue->type.structName);
		size_t memorySize = structDef.memorySize;

		IR_StructInit structInit;
		structInit.byteNum = (int)memorySize;

		//TODO: FINISH THIS AND DETERMINE HOW TO TREAT STRUCTS
	}
	else
	{
		IR_Value value;
		value.varIndex = irState.state.stackVarIndex++;
		value.valueType = IR_VARIABLE;
		value.type = (lvalue->type.pointerLevel > 0 || lvalue->type.lValueType == LValueType::INT) ? IR_INT : IR_FLOAT;
		//value.varIndex = irState.state.stackVarIndex++; //decide how to do this when assigning to expression to prevent lots of duplicate copying
		value.byteSize = (lvalue->type.pointerLevel > 0) ? POINTER_SIZE : 4;
		value.isTempValue = false;
		irState.state.scope.variableMapping.back()[lvalue->name] = value; //add to variable dictionary
		
		irState.IR_statements.push_back(make_unique<IR_VariableInit>(IR_VariableInit(value)));

		if (rvalue)
		{
			//add assign statement afterwards
			IR_Value irRValue = rvalue->ConvertExpressionToIR(irState);
			if (irRValue.specialVars == IR_FLAGS)
			{
				irState.IR_statements.push_back(make_unique<IR_Assign>(IR_Assign(irRValue.type, IR_FLAG_CONVERT, IR_Operand(value), IR_Operand(irRValue))));
			}
			else {
				irState.IR_statements.push_back(make_unique<IR_Assign>(IR_Assign(irRValue.type, IR_COPY, IR_Operand(value), IR_Operand(irRValue))));
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

//void ParseConditionalIR(unique_ptr<Expression>& Condition, IR& irState, int& labelEnd)
//{
//	if (Condition->GetExpressionType() == _BinOp)
//	{
//		//parse AND + OR, as well as comparisons; other bin ops don't matter
//
//		AST_BinOp* binOpExpr = ExpressionFunctions::GetSubexpressionPtr<AST_BinOp>(Condition);
//		if (binOpExpr->op == BinOpType::AND || binOpExpr->op == BinOpType::OR)
//		{
//			//IMPORTANT: BECAUSE OF SHORT CIRCUITING, IT MUST JUMP AFTER CONDITION THAT WOULD SHORT CIRCUIT IT
//			//i.e. for the statement: if(x && y) -> if x is false, you have to jump; you can't evaluate y
//		}
//		else if (IsComparisonOperation(binOpExpr->op))
//		{
//			IR_Value op1 = binOpExpr->left->ConvertExpressionToIR(irState);
//			IR_Value op2 = binOpExpr->right->ConvertExpressionToIR(irState);
//
//			irState.IR_statements.push_back(make_unique<IR_Compare>(IR_Compare(IR_Operand(op1), IR_Operand(op2))));
//
//			IR_FlagResults cmpFlag;
//			switch (binOpExpr->op)
//			{
//				case BinOpType::NOT_EQUALS: cmpFlag = IR_FlagResults::IR_NOT_EQUALS;
//				case BinOpType::EQUALS: cmpFlag = IR_FlagResults::IR_EQUALS;
//				case BinOpType::GREATER: cmpFlag = IR_FlagResults::IR_GREATER;
//				case BinOpType::GREATER_EQUAL: cmpFlag = IR_FlagResults::IR_GREATER_EQUALS;
//				case BinOpType::LESS: cmpFlag = IR_FlagResults::IR_LESS;
//				case BinOpType::LESS_EQUAL: cmpFlag = IR_FlagResults::IR_LESS_EQUALS;
//			}
//
//
//		}
//		else {
//			//TODO: treat like normal value (is it zero or not)
//		}
//	}
//	else
//	{
//		//for non-binary expressions, return if value is not equal to 0
//		IR_Value value = Condition->ConvertExpressionToIR(irState);
//		const IR_Value zero(value.type, IR_LITERAL, value.byteSize, true, "0");
//		irState.IR_statements.push_back(make_unique<IR_Compare>(IR_Compare(IR_Operand(value), IR_Operand(zero))));
//
//		//create label for not true (at bottom)
//		//Add JNE statement
//	}
//}

void AST_If_Then::ConvertStatementToIR(IR& irState)
{
	if (Condition->GetExpressionType() == _BinOp)
	{
		//parse AND + OR, as well as comparisons; other bin ops don't matter
		
		AST_BinOp* binOpExpr = ExpressionFunctions::GetSubexpressionPtr<AST_BinOp>(Condition);
		if (binOpExpr->op == BinOpType::AND || binOpExpr->op == BinOpType::OR)
		{
			//IMPORTANT: BECAUSE OF SHORT CIRCUITING, IT MUST JUMP AFTER CONDITION THAT WOULD SHORT CIRCUIT IT
			//i.e. for the statement: if(x && y) -> if x is false, you have to jump; you can't evaluate y
		}
		else if (IsComparisonOperation(binOpExpr->op))
		{
			IR_Value op1 = binOpExpr->left->ConvertExpressionToIR(irState);
			IR_Value op2 = binOpExpr->right->ConvertExpressionToIR(irState);
		
			irState.IR_statements.push_back(make_unique<IR_Compare>(IR_Compare(IR_Operand(op1), IR_Operand(op2))));
		
			switch (binOpExpr->op)
			{
				//case BinOpType::NOT_EQUALS: return IR_FlagResults::NOT_EQUALS;
				//case BinOpType::EQUALS: return IR_FlagResults::EQUALS;
				//case BinOpType::GREATER: return IR_FlagResults::GREATER;
				//case BinOpType::GREATER_EQUAL: return IR_FlagResults::GREATER_EQUALS;
				//case BinOpType::LESS: return IR_FlagResults::LESS;
				//case BinOpType::LESS_EQUAL: return IR_FlagResults::LESS_EQUALS;
			}
		}
	}
	else
	{
		//for non-binary expressions, return if value is not equal to 0
		IR_Value value = Condition->ConvertExpressionToIR(irState);
		const IR_Value zero(value.type, IR_LITERAL, value.byteSize, true, "0");
		irState.IR_statements.push_back(make_unique<IR_Compare>(IR_Compare(IR_Operand(value), IR_Operand(zero))));
				
		//create label for not true (at bottom)
		//Add JNE statement
	}

	int notTrueLabelIdx = irState.state.labelIndex++;

	irState.EnterScope();
	this->ifStatement->ConvertStatementToIR(irState); //convert statements inside if to IR
	irState.ExitScope();

	for (const unique_ptr<AST_Else_If>& elseIf : elseIfStatements)
	{
		irState.EnterScope();
		//parse statements and have proper branching code
		elseIf->ConvertStatementToIR(irState);
		irState.ExitScope();
	}
	if (this->elseStatement)
	{
		irState.EnterScope();
		int endLabelIdx = irState.state.labelIndex++;
		irState.IR_statements.push_back(make_unique<IR_Jump>(IR_Jump(endLabelIdx, IR_ALWAYS)));
		//irState.IR_statements.push_back(make_unique<IR_Label>(IR_Label(notTrueLabelIdx))); //watch out for ELSE-IF STATEMENTS!!!!

		this->elseStatement->ConvertStatementToIR(irState);
		irState.IR_statements.push_back(make_unique<IR_Label>(IR_Label(notTrueLabelIdx)));
		irState.ExitScope();
	}

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