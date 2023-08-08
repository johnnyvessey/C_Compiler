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
		AST_Struct_Definition structDefinition;
		bool foundStruct = scopeStack.TryFindStructName(token.value, structDefinition.def);
		assert(foundStruct, "Struct " + token.value + " does not exist in this scope", GetCurrentLineNum());
		varType.structName = structDefinition.def.name;
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

	Function func;
	GetFunctionReturnType(func.def.returnType);
	Token& functionNameToken = GetCurrentToken();
	func.def.name = functionNameToken.value;

	assert(!scopeStack.FunctionNameExists(functionNameToken.value), "Function name already defined", GetCurrentLineNum());

	currentIndex += 2;
	func.def.arguments = ParseFunctionParameters();
	// 
	// expect close parentheses, add to index, etc...
	ScopeLevel level;
	for (const Variable& var : func.def.arguments)
	{
		level.variables[var.name] = var;
	}
	scopeStack.scope.push_back(std::move(level));

	//add to function scope before parsing statements to allow for recursion
	scopeStack.functionScope[func.def.name] = func.def;

	func.statements = ParseFunctionStatements(func.def.returnType);

	//Don't store statements in scope
	//update function in scope dictionary to include statements
	//scopeStack.functionScope[func.def.name].statements = func.statements;

	scopeStack.scope.pop_back();

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
	int scopeLevel = scopeStack.scope.size();

	assert(GetCurrentToken().type == TokenType::OPEN_BRACE, "Open brace required for function statement", GetCurrentLineNum());
	++currentIndex;

	scopeStack.scope.push_back(std::move(ScopeLevel()));
	//TODO: figure out if this could ever be infinite loop
	while (GetCurrentToken().type != TokenType::CLOSE_BRACE)
	{
		unique_ptr<Statement> statement = ParseStatement();

		AssertMatchingReturnType(returnType, statement);

		statementGroupPtr->statements.push_back(std::move(statement));

	}
	scopeStack.scope.pop_back();
	++currentIndex;

	return std::move(statementGroupPtr);
}

unique_ptr<AST_Function_Expression> AST::ParseFunctionCall(Function& f)
{
	currentIndex += 2;

	AST_Function_Expression funcExpr;
	funcExpr.type = f.def.returnType;
	funcExpr.argumentInstances.reserve(f.def.arguments.size());
	funcExpr.functionName = f.def.name;

	int argIndex = 0;
	while (GetCurrentToken().type != TokenType::CLOSE_PAR)
	{
		unique_ptr<Expression> arg = ParseExpression();
		assert(arg->type == f.def.arguments.at(argIndex).type, "Type mismatch on " + std::to_string(argIndex) + " argument of function", GetCurrentLineNum());
		funcExpr.argumentInstances.push_back(std::move(arg));
		++argIndex;

		Token& token = GetCurrentToken();
		if (token.type == TokenType::COMMA)
		{
			++currentIndex;
		} 
		else if (token.type != TokenType::CLOSE_PAR)
		{
			throwError("Invalid syntax in function arguments", GetCurrentLineNum());
		}
	}

	assert(GetCurrentToken().type == TokenType::CLOSE_PAR, "Function call must have closing parentheses", GetCurrentLineNum());
	++currentIndex;


	return make_unique<AST_Function_Expression>(std::move(funcExpr));
}

void AST::AssertMatchingReturnType(VariableType& returnType, unique_ptr<Statement>& statement)
{
	AST_Return_Statement* retStatement = dynamic_cast<AST_Return_Statement*>(statement.get());

	if (retStatement)
	{
		if (retStatement->returnExpression) {
			assert(retStatement->returnExpression->type == returnType, "Return type does not match function type", GetCurrentLineNum());
		}
		else {
			assert(returnType.lValueType == LValueType::VOID, "Must return value in non-void function", GetCurrentLineNum());
		}
	}
}
