#include "Parser.h"


unique_ptr<AST_Initialization> AST::ParseInitAssignment()
{
	//TODO: Include function assignment [REMOVE ABILITY TO DECLARE VARIABLES LIKE int x(2);]
	AST_Initialization assignment;

	Variable v;
	v.type.lValueType = GetTypeFromName(std::move(tokens.at(currentIndex).value));
	v.type.structName = v.type.lValueType == LValueType::STRUCT ? tokens.at(currentIndex).value : "";

	++currentIndex;
	v.type.pointerLevel = GetConsecutiveTokenNumber(TokenType::STAR);

	Token& token = GetCurrentToken();
	assert(token.type == TokenType::NAME, "Improper variable assignment", token.lineNumber);

	v.name = token.value;
	++currentIndex;
	
	assignment.lvalue = make_unique<Variable>(v);

	//TODO: FIGURE OUT HOW TO PARSE ARRAY
	ScopeLevel& top = scopeStack.scope.back();
	assert(top.variables.find(v.name) == top.variables.end(), "Variable is already defined in this scope", GetCurrentLineNum());
	top.variables[v.name] = v;

	if (tokens.at(currentIndex).type == TokenType::SINGLE_EQUAL)
	{
		++currentIndex;
		unique_ptr<Expression> expr = ParseExpression();
		assert(expr->type == v.type, "Variable is assigned to the wrong type", tokens.at(currentIndex).lineNumber);
		assignment.rvalue = std::move(expr);
	}

	assert(tokens.at(currentIndex).type == TokenType::SEMICOLON, "Semicolon required at the end of this line", tokens.at(currentIndex).lineNumber);
	++currentIndex;

	return make_unique<AST_Initialization>(std::move(assignment));
}

//TODO: Figure out if assignment and initialization should be different in AST
unique_ptr<AST_Assignment> AST::ParseAssignment(unique_ptr<LValueExpression>&& lValueExpr)
{
	//Token& token = tokens.at(currentIndex);
	/*Variable var;
	bool foundVariable = scopeStack.TryFindVariable(token.value, var);
	if (!foundVariable) {
		throwError("Variable doesn't exist in scope", token.lineNumber);
	}*/


	AST_Assignment assignment;

	assignment.lvalue = std::move(lValueExpr);
	Token& opToken = tokens.at(currentIndex);

	++currentIndex;
	//if (Lexer::IsBinaryAssignmentOp(opToken.type)) // +=, -=. *=, /=, %=
	//{
	//	//AST_Variable_Expression varExpr(var);
	//	AST_BinOp binOp;
	//	binOp.op = ExpressionUtils::BinOpAssignmentTypeDictionary.at(opToken.type);
	//	binOp.left = make_unique<AST_Variable_Expression>(std::move());
	//	binOp.right = ParseExpression();

	//	//do type check!!!

	//	assignment.rvalue = make_unique<AST_BinOp>(std::move(binOp));
	//}
	//else
	//{
	//	assignment.rvalue = ParseExpression();
	//}

	assignment.assignmentOperator = opToken.type;
	assignment.rvalue = ParseExpression();


	assert(assignment.lvalue->type == assignment.rvalue->type, "Type mismatch in variable assignment", opToken.lineNumber);

	assert(GetCurrentToken().type == TokenType::SEMICOLON, "Missing semicolon", GetCurrentLineNum());
	++currentIndex;
	return make_unique<AST_Assignment>(std::move(assignment));

}