#include "Parser.h"


AST_Struct_Definition AST::ParseStructDefinition()
{
	string structName = tokens.at(currentIndex + 1).value;

	assert(tokens.at(currentIndex + 2).type == TokenType::OPEN_BRACE, "Struct definition requires open brace", tokens.at(currentIndex).lineNumber);
	currentIndex += 3;

	StructDefinition structDef;
	assert(!scopeStack.TryFindStructName(structName, structDef), "Struct already defined", GetCurrentLineNum()); //can't redefine struct

	structDef.name = structName;
	//add to scope so that pointers to this struct can be variables of the struct
	scopeStack.scope.back().structs[structName] = structDef;

	unordered_map<string, Struct_Variable> structVariables;
	vector<Struct_Variable> structVariablesVector;

	size_t currentMemoryOffset = 0;

	while (GetCurrentToken().type != TokenType::CLOSE_BRACE)
	{
		Token& token = GetCurrentToken();
		Variable v;
		Struct_Variable structVar;

		if (token.type == TokenType::STRUCT)
		{
			++currentIndex;
			v.type.lValueType = LValueType::STRUCT;
			
			Token& structNameToken = GetCurrentToken();
			assert(structNameToken.type == TokenType::NAME, "Must include struct name", token.lineNumber);

			AST_Struct_Definition structDef;
			v.type.structName = structNameToken.value;

			assert(scopeStack.TryFindStructName(structNameToken.value, structDef.def), "Struct name (" + structNameToken.value + ") not found in scope", GetCurrentLineNum());
		
		}
		else if (token.type == TokenType::TYPE)
		{
			v.type.lValueType = token.value == "int" ? LValueType::INT : LValueType::FLOAT;
		}
		else {
			throwError("Invalid struct definition syntax", token.lineNumber);
		}

		++currentIndex;
		v.type.pointerLevel = GetConsecutiveTokenNumber(TokenType::STAR);

		assert(!(v.type.lValueType == LValueType::STRUCT && v.type.structName == structName && v.type.pointerLevel == 0),
			"Can't have non-pointer variable of same struct type as member variable of the struct", GetCurrentLineNum());

		Token& varNameToken = GetCurrentToken();
		assert(varNameToken.type == TokenType::NAME, "Must include variable name", varNameToken.lineNumber);
		v.name = varNameToken.value;

		assert(structVariables.find(v.name) == structVariables.end(), "Variable name " + v.name + " already used in this struct definition", GetCurrentLineNum());

		++currentIndex;
		assert(GetCurrentToken().type == TokenType::SEMICOLON, "Variable declaration must end with semicolon", GetCurrentLineNum());
		++currentIndex;

		structVar.memoryOffset = currentMemoryOffset;
		structVar.v = std::move(v);

		currentMemoryOffset += GetMemorySize(structVar.v.type);

		structVariablesVector.push_back(structVar);
		structVariables[structVar.v.name] = std::move(structVar);
		
	}

	AST_Struct_Definition structDefinition = AST_Struct_Definition(structName, std::move(structVariables), std::move(structVariablesVector), currentMemoryOffset);
	scopeStack.scope.back().structs[structName] = structDefinition.def;

	++currentIndex;
	assert(GetCurrentToken().type == TokenType::SEMICOLON, "Struct definition must end with semicolon", GetCurrentLineNum());
	++currentIndex;

	return structDefinition;
}

//Statement ParseStructVariableInitialization()
//{

//}

size_t AST::GetMemorySize(VariableType type)
{
	if (type.pointerLevel > 0)
	{
		return 8;
	}
	else if (type.lValueType == LValueType::INT || type.lValueType == LValueType::FLOAT || type.lValueType == LValueType::BOOL)
	{
		return 4;
	}
	else if (type.lValueType == LValueType::STRUCT)
	{
		StructDefinition structDef;
		scopeStack.TryFindStructName(type.structName, structDef);

		return structDef.memorySize;
	}
	else {
		throwError("Can't find memory size of this object", GetCurrentLineNum());
	}

	
}


unique_ptr<AST_Struct_Variable_Access> AST::ParseStructVariableAccess(unique_ptr<Expression>&& expr)
{
	assert(expr->type.lValueType == LValueType::STRUCT, "Cannot use period to access subvariable of non-struct", GetCurrentLineNum());

	unique_ptr<AST_Struct_Variable_Access> structAccessExpr = make_unique<AST_Struct_Variable_Access>();

	structAccessExpr->type = expr->type;

	StructDefinition structDef;
	assert(scopeStack.TryFindStructName(expr->type.structName, structDef), "Struct definition does not exist in scope", GetCurrentLineNum());


	++currentIndex;
	Token& varNameToken = GetCurrentToken();
	assert(varNameToken.type == TokenType::NAME && structDef.variableMapping.find(varNameToken.value) != structDef.variableMapping.end(), 
		"Can't find variable (" + varNameToken.value + ") in struct type (" + structDef.name + ")", varNameToken.lineNumber);

	structAccessExpr->type = structDef.variableMapping.at(varNameToken.value).v.type;
	structAccessExpr->varName = varNameToken.value;
	structAccessExpr->expr = std::move(expr);

	++currentIndex;

	return structAccessExpr;
}