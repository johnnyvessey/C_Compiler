#include "Parser.h"

AST_Struct_Definition AST::ParseStructDefinition()
{
	assert(tokens.at(currentIndex + 2).type == TokenType::OPEN_BRACE, "Struct definition requires open brace", tokens.at(currentIndex).lineNumber);
	vector<Struct_Variable> structVariables;

	//TODO: FINISH THIS

	return AST_Struct_Definition("NO NAME", vector<Struct_Variable>());
}

//Statement ParseStructVariableInitialization()
//{

//}