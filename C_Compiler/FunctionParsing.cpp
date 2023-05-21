#include "Parser.h"


unique_ptr<AST_Function_Definition> AST::ParseFunctionDefinition()
{
	assert(scopeStack.scope.size() == 1, "Function must be defined at global level", GetCurrentLineNum());

	Token& functionNameToken = tokens.at(currentIndex + 1);
	assert(!scopeStack.FunctionNameExists(functionNameToken.value), "Function name already defined", GetCurrentLineNum());
	Function func;
	func.name = functionNameToken.value;
	//func.returnType = GetFunctionReturnType(tokens.at(currentIndex).type);
	func.arguments = ParseFunctionParameters();
	// 
	// expect close parentheses, add to index, etc...
	ScopeLevel level;
	for (const Variable& var : func.arguments)
	{
		level.variables[var.name] = var;
	}
	scopeStack.scope.push_back(std::move(level));

	//func.statements = ParseFunctionStatments();

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
			v.type = LValueType::STRUCT;
			++currentIndex;
			assert(GetCurrentToken().type == TokenType::NAME, "Incorrect parameter syntax for struct type", GetCurrentLineNum());
			v.structName = GetCurrentToken().value;;
		}
		else {
			Token& token = GetCurrentToken();
			assert(token.type == TokenType::TYPE, "Function parameter must have variable type", GetCurrentLineNum());
			v.type = GetTypeFromName(std::move(token.value));
		}

		v.pointerLevel = GetConsecutiveTokenNumber(TokenType::STAR);
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
