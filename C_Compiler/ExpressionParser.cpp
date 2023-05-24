#include "Parser.h"


unique_ptr<Expression> AST::ParseExpression()
{
	unique_ptr<Expression> firstExpr;
	if (tokens.at(currentIndex).type == TokenType::OPEN_PAR)
	{
		++currentIndex;
		firstExpr = ParseParentheticalExpression();
	}
	else
	{
		firstExpr = ParseNonBinaryExpression();
	}
	Token& token = tokens.at(currentIndex);

	if (Lexer::IsBinOp(token.type))
	{
		assert(firstExpr->type.lValueType != LValueType::STRUCT || firstExpr->type.pointerLevel > 0, "Cannot perform binary operation on non-pointer struct type", token.lineNumber);

		return ParseBinaryExpression(std::move(firstExpr));
	}
	else
	{
		return firstExpr;
	}
}


unique_ptr<Expression> AST::ParseNonBinaryExpression()
{
	unique_ptr<Expression> expr;
	Token& token = tokens.at(currentIndex);
	Token& nextToken = tokens.at(currentIndex + 1);

	if (token.type == TokenType::OPEN_PAR)
	{
		++currentIndex;
		return ParseParentheticalExpression();
	}
	else if (Lexer::IsUnaryOp(token.type)) //prefix unary op
	{

	}
	else if (token.type == TokenType::STAR) //dereferencing pointer
	{
		++currentIndex;
		return ParsePointerDereferenceExpression();
	}
	else if (token.type == TokenType::NAME)
	{
		//FIND IF FUNCTION
		// 
		//FIND IF VARIABLE
		if (nextToken.type == OPEN_PAR) //function
		{
			Function f;
			bool foundFunction = scopeStack.TryFindFunction(token.value, f);
			assert(foundFunction, "Function: " + token.value + " doesn't exist in scope", token.lineNumber);

		}
		else //variable
		{
			Variable v;
			bool foundVariable = scopeStack.TryFindVariable(token.value, v);
			assert(foundVariable, "Variable: " + token.value + " doesn't exist in scope", token.lineNumber);

			if (nextToken.type == TokenType::PERIOD)  //struct variable access
			{

			}
			else if (Lexer::IsBinOp(nextToken.type) || nextToken.type == TokenType::SEMICOLON || nextToken.type == TokenType::COMMA || nextToken.type == TokenType::CLOSE_PAR) //end of non-binary expression
			{
				++currentIndex;
				AST_Variable_Expression expr = AST_Variable_Expression(v);
				return make_unique<AST_Variable_Expression>(std::move(expr));
			}
			else if (Lexer::IsUnaryOp(nextToken.type)) //postfix unary operation
			{

			}
			else if (nextToken.type == TokenType::ARROW) //dereference struct pointer
			{

			}
			else if (nextToken.type == TokenType::OPEN_BRACKET) //accessing array element
			{

			}
			else {
				assert(false, "Invalid syntax for expression", nextToken.lineNumber);
			}
		}
	}
	else if (token.type == TokenType::INT_LITERAL || token.type == TokenType::FLOAT_LITERAL)
	{

		if (nextToken.type == TokenType::SEMICOLON || nextToken.type == TokenType::COMMA || nextToken.type == TokenType::CLOSE_PAR || Lexer::IsBinOp(nextToken.type))
		{
			currentIndex += 1; //(nextToken.type == TokenType::SEMICOLON ? 2 : 1); //if semicolon, continue to next statement. Otherwise, let comma/close_par be parsed after return
			AST_Literal_Expression expr;

			//expand this line to fit other literal types??
			expr.type.lValueType = token.type == TokenType::INT_LITERAL ? LValueType::INT : LValueType::FLOAT;
			expr.value = token.value;
			expr.type.pointerLevel = 0;

			return make_unique<AST_Literal_Expression>(std::move(expr));;
		}
		/*else if (IsBinOp(nextToken.type))
		{

		}*/
		else {
			throwError("Invalid syntax", token.lineNumber);
		}
	}
	else if (token.type == TokenType::STAR) //pointer dereferencing
	{

	}

	return expr; //CHANGE THIS!!!!
}

unique_ptr<AST_BinOp> AST::ParseBinaryExpression(unique_ptr<Expression> firstExpr)
{
	const BinOp& op = ExpressionUtils::BinOpTokenDictionary.at(tokens.at(currentIndex).type);
	++currentIndex;

	unique_ptr<Expression> secondExpr = ParseNonBinaryExpression();

	//TODO: Check to make sure types of first and second expression match (unless first type is pointer and second is int)
	Token& currentToken = tokens.at(currentIndex);

	unique_ptr<AST_BinOp> binOpExpr = make_unique<AST_BinOp>();
	binOpExpr->left = std::move(firstExpr);
	binOpExpr->op = op.type;

	binOpExpr->type.lValueType = IsBooleanOperation(op.type) ? LValueType::BOOL : binOpExpr->left->type.lValueType;

	if (Lexer::IsBinOp(currentToken.type))
	{
		const BinOp& secondOp = ExpressionUtils::BinOpTokenDictionary.at(currentToken.type);
		if (op.precedence >= secondOp.precedence)
		{
			binOpExpr->right = std::move(secondExpr);

			//TODO: Add casting from int to float if one value is int and the other is float; ALSO, do BOOL conversions to INT/FLOAT, etc...
			assert((IsNumericType(binOpExpr->left->type.lValueType) && IsNumericType(binOpExpr->right->type.lValueType))
				|| (binOpExpr->left->type.pointerLevel > 0 && binOpExpr->right->type.lValueType == LValueType::INT && binOpExpr->right->type.pointerLevel == 0),
				"Type mismatch in binary operation", currentToken.lineNumber);

			return ParseBinaryExpression(std::move(binOpExpr));
		}
		else
		{
			binOpExpr->right = ParseBinaryExpression(std::move(secondExpr));
		}
	}
	else
	{
		binOpExpr->right = std::move(secondExpr);
	}

	assert(binOpExpr->left->type == binOpExpr->right->type
		|| (binOpExpr->left->type.pointerLevel > 0 && binOpExpr->right->type.lValueType == LValueType::INT && binOpExpr->right->type.pointerLevel == 0),
		"Type mismatch in binary operation", currentToken.lineNumber);

	return binOpExpr;
}


unique_ptr<Expression> AST::ParseParentheticalExpression()
{
	unique_ptr<Expression> expr = ParseExpression();
	Token& token = tokens.at(currentIndex);
	assert(token.type == TokenType::CLOSE_PAR, "Invalid parentheses", token.lineNumber);
	++currentIndex;

	return expr;
}

unique_ptr<Expression> AST::ParsePointerDereferenceExpression()
{
	unique_ptr<Expression> expr = ParseExpression();
	assert(expr->type.pointerLevel > 0, "Can't dereference a non-pointer type", GetCurrentLineNum());
	--expr->type.pointerLevel;

	return std::move(expr);
}

