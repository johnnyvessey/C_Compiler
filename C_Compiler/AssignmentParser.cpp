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
	

	//TODO: FIGURE OUT HOW TO PARSE ARRAY
	ScopeLevel& top = scopeStack.scope.back();
	assert(top.variables.find(v.name) == top.variables.end(), "Variable is already defined in this scope", GetCurrentLineNum());

	if (GetCurrentToken().type == TokenType::OPEN_BRACKET)
	{
		++v.type.pointerLevel; //array will have pointer level of 1 more than its elements' types

		//parse array initialization
		++currentIndex;
		Token& curToken = GetCurrentToken();
		assert(curToken.type == TokenType::INT_LITERAL, "Must have int literal for array size", curToken.lineNumber); //don't have constexpr's yet so couldn't do int x[2 + 3];
		//potentially could add in inferred length (i.e. int x[] = {1,2,3};
		v.arraySize = stoi(curToken.value);
		assert(v.arraySize > 0, "Array length must be positive", curToken.lineNumber);

		++currentIndex;
		assert(GetCurrentToken().type == TokenType::CLOSE_BRACKET, "Array declaration must have close bracket", GetCurrentLineNum());

		++currentIndex;
	}

	if (GetCurrentToken().type == TokenType::SINGLE_EQUAL)
	{
		++currentIndex;
		if (GetCurrentToken().type == TokenType::OPEN_BRACE && (v.type.lValueType == LValueType::STRUCT || v.arraySize > 0))
		{
			++currentIndex;
			//unique_ptr<Expression> expr = ParseCurlyBraceInitializationExpression();
			//have new statement that is group assignment
				//it contains a vector of assignments
				//recursively go through struct variables to get offset of every leaf variable (non struct type) [if type is curly brace]
				//assign offset to each variable (offset from start of struct in memory)
			/*
				struct ex1 {
					int x;
					int y;
				};

				struct ex2{
					int x2;
					struct ex1 v;
					struct ex1 v2;
				}

				struct ex2 var = {1, {1,2}, {2,3}};

				becomes:

				(offset 4, len 4) -> 1
				(offset 8, len 4) -> 1
				(offset 12, len 4) -> 2
				*/
			//assignment.rvalue = std::move(expr);
			 
		}
		else
		{
			unique_ptr<Expression> expr = ParseExpression();
			assert(expr->type == v.type, "Variable is assigned to the wrong type", tokens.at(currentIndex).lineNumber);
			assignment.rvalue = std::move(expr);

		}
	}
	

	assignment.lvalue = make_unique<Variable>(v);
	top.variables[v.name] = v;


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