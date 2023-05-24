#include "Parser.h"

void AST::GetFunctionReturnType(VariableType& varType)
{
	Token& token = GetCurrentToken();
	if (token.type == TYPE)
	{
		varType.lValueType = (token.value == "int") ? LValueType::INT : LValueType::FLOAT;
	}
	else if (token.type == TokenType::VOID)
	{
		varType.lValueType = LValueType::VOID;
	}
	else if (token.type == TokenType::NAME)
	{
		AST_Struct_Definition structDef;
		bool foundStruct = scopeStack.TryFindStructName(token.value, structDef);
		assert(foundStruct, "Struct " + token.value + " does not exist in this scope", GetCurrentLineNum());
		varType.structName = structDef.name;
		varType.lValueType = LValueType::STRUCT;
	}
	else {
		throwError("Invalid function return type", GetCurrentLineNum());
	}

	++currentIndex;
	varType.pointerLevel = GetConsecutiveTokenNumber(TokenType::STAR);
}


unique_ptr<AST_Function_Definition> AST::ParseFunctionDefinition()
{
	assert(scopeStack.scope.size() == 1, "Function must be defined at global level", GetCurrentLineNum());

	Token& functionNameToken = tokens.at(currentIndex + 1);
	assert(!scopeStack.FunctionNameExists(functionNameToken.value), "Function name already defined", GetCurrentLineNum());
	Function func;
	func.name = functionNameToken.value;
	GetFunctionReturnType(func.returnType);

	currentIndex += 2;
	func.arguments = ParseFunctionParameters();
	// 
	// expect close parentheses, add to index, etc...
	ScopeLevel level;
	for (const Variable& var : func.arguments)
	{
		level.variables[var.name] = var;
	}
	scopeStack.scope.push_back(std::move(level));

	func.statements = ParseFunctionStatements(func.returnType);

	scopeStack.scope.pop_back();
	scopeStack.functionScope[func.name] = func;

	AST_Function_Definition funcDef;
	funcDef.func = make_unique<Function>(std::move(func));

	return make_unique<AST_Function_Definition>(std::move(funcDef));
}


vector<Variable> AST::ParseFunctionParameters()
{
	vector<Variable> params;

	while (GetCurrentToken().type != TokenType::CLOSE_PAR)
	{
		Variable v;
		if (GetCurrentToken().type == TokenType::STRUCT)
		{
			v.type.lValueType = LValueType::STRUCT;
			++currentIndex;
			assert(GetCurrentToken().type == TokenType::NAME, "Incorrect parameter syntax for struct type", GetCurrentLineNum());
			v.type.structName = GetCurrentToken().value;;
		}
		else {
			Token& token = GetCurrentToken();
			assert(token.type == TokenType::TYPE, "Function parameter must have variable type", GetCurrentLineNum());
			v.type.lValueType = GetTypeFromName(std::move(token.value));
		}

		++currentIndex;
		v.type.pointerLevel = GetConsecutiveTokenNumber(TokenType::STAR);

		assert(GetCurrentToken().type == TokenType::NAME, "Function parameter must have variable name", GetCurrentLineNum());
		v.name = GetCurrentToken().value;
		++currentIndex;

		params.push_back(std::move(v));

		Token& token = GetCurrentToken();

		if (token.type == CLOSE_PAR)
		{
			++currentIndex;
			break;
		}
		else if (token.type != COMMA)
		{
			throwError("Comma required after argument", GetCurrentLineNum());
		}

		++currentIndex;
	}

	return params;
}

unique_ptr<StatementGroup> AST::ParseFunctionStatements(VariableType& returnType)
{
	unique_ptr<StatementGroup> statementGroupPtr = make_unique<StatementGroup>();
	size_t scopeLevel = scopeStack.scope.size();

	assert(GetCurrentToken().type == TokenType::OPEN_BRACE, "Open brace required for function statement", GetCurrentLineNum());
	++currentIndex;

	scopeStack.scope.push_back(std::move(ScopeLevel()));
	//TODO: figure out if this could ever be infinite loop
	while (GetCurrentToken().type != TokenType::CLOSE_BRACE)
	{
		unique_ptr<Statement> statement = ParseStatement();
		statementGroupPtr->statements.push_back(std::move(statement));

		/*AST_Return_Statement* retStatement = dynamic_cast<AST_Return_Statement*>(statement.get());

		if (retStatement)
		{
			assert(retStatement->returnExpression->type == returnType, "Return type does not match function type", GetCurrentLineNum());
		}*/

		

	}
	scopeStack.scope.pop_back();
	++currentIndex;

	return std::move(statementGroupPtr);
}
