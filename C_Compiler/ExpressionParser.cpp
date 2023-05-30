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


unique_ptr<LValueExpression> AST::ConvertExpressionToLValue(unique_ptr<Expression>&& expr)
{
	assert(expr->isLValue, "Can only use unary assignment operator on LValue", GetCurrentLineNum());

	LValueExpression* tmp = dynamic_cast<LValueExpression*>(expr.get());
	unique_ptr<LValueExpression> lValueExpr;
	if (tmp != nullptr)
	{
		expr.release();
		lValueExpr.reset(tmp);
	}
	else
	{
		throwError("Error converting expression to LValue type", GetCurrentLineNum());
	}

	return lValueExpr;
}

unique_ptr<LValueExpression> AST::ParseLValueExpression()
{
	unique_ptr<Expression> expr = AST::ParseNonBinaryExpression();
	return ConvertExpressionToLValue(std::move(expr));
}

unique_ptr<Expression> AST::ParseNonBinaryExpression(unique_ptr<Expression> prev)
{
	unique_ptr<Expression> expr;
	Token& token = tokens.at(currentIndex);
	Token& nextToken = tokens.at(currentIndex + 1);

	//end of non-binary expression
	if (Lexer::IsNonBinaryExpressionTerminalToken(token.type))
	{
		assert(prev != nullptr, "Expression is empty", token.lineNumber);
		return prev;
	}
	else if (token.type == TokenType::OPEN_PAR)
	{
		++currentIndex;
		return ParseNonBinaryExpression(ParseParentheticalExpression());
	}
	else if (Lexer::IsUnaryAssignmentOp(token.type)) //prefix unary op
	{
		unique_ptr<AST_Unary_Assignment_Expression> unaryExpr = make_unique<AST_Unary_Assignment_Expression>();
		unaryExpr->opType = token.type;

		if (!prev)
		{
			++currentIndex;
			unaryExpr->expr = ParseLValueExpression();
		}
		else
		{
			unaryExpr->expr = ConvertExpressionToLValue(std::move(prev));
			assert(Lexer::IsNonBinaryExpressionTerminalToken(nextToken.type), "Postfix unary operator must be the end of the expression", nextToken.lineNumber);
		}

		return unaryExpr;
	}
	else if (token.type == TokenType::STAR) //dereferencing pointer, right-to-left associativity
	{
		++currentIndex;
		return ParsePointerDereferenceExpression();
	}
	else if (token.type == TokenType::NAME)
	{
		if (nextToken.type == OPEN_PAR) //function
		{
			assert(!prev, "Invalid function syntax", GetCurrentLineNum());
			Function f;
			bool foundFunction = scopeStack.TryFindFunction(token.value, f);
			assert(foundFunction, "Function: " + token.value + " doesn't exist in scope", token.lineNumber);

			 unique_ptr<AST_Function_Expression> expr = ParseFunctionCall(f);
			 return ParseNonBinaryExpression(std::move(expr));

		}
		else //variable
		{
			if (!prev) //only check for variable if it's the first part of the expression (not if it's struct variable access, etc...)
			{
				Variable v;
				bool foundVariable = scopeStack.TryFindVariable(token.value, v);
				assert(foundVariable, "Variable: " + token.value + " doesn't exist in scope", token.lineNumber);

				++currentIndex;
				return ParseNonBinaryExpression(make_unique<AST_Variable_Expression>(std::move(v)));
			}
			else //acessing struct variables will be part of the process when parsing periods, arrows, brackets, etc...
			{
				throwError("Invalid syntax with name", GetCurrentLineNum());
			}
		}
	}
	else if (token.type == TokenType::PERIOD)  //struct variable access
	{
		unique_ptr<AST_Struct_Variable_Access> expr = ParseStructVariableAccess(std::move(prev));
		return ParseNonBinaryExpression(std::move(expr));
	}
	else if (nextToken.type == TokenType::ARROW) //dereference struct pointer
	{
		unique_ptr<AST_Pointer_Dereference> ptrDerefExpr = make_unique<AST_Pointer_Dereference>();
		ptrDerefExpr->baseExpr = std::move(prev);
		return ParseStructVariableAccess(std::move(ptrDerefExpr));
	}
	else if (nextToken.type == TokenType::OPEN_BRACKET) //accessing array element
	{

	}
	else if (token.type == TokenType::INT_LITERAL || token.type == TokenType::FLOAT_LITERAL)
	{

		if (Lexer::IsNonBinaryExpressionTerminalToken(nextToken.type))
		{
			currentIndex += 1; //(nextToken.type == TokenType::SEMICOLON ? 2 : 1); //if semicolon, continue to next statement. Otherwise, let comma/close_par be parsed after return
			AST_Literal_Expression expr;

			//expand this line to fit other literal types??
			expr.type.lValueType = token.type == TokenType::INT_LITERAL ? LValueType::INT : LValueType::FLOAT;
			expr.value = token.value;
			expr.type.pointerLevel = 0;

			return make_unique<AST_Literal_Expression>(std::move(expr));;
		}
		else {
			throwError("Invalid syntax", token.lineNumber);
		}
	}
	else
	{
		throwError("Invalid sytnax", GetCurrentLineNum());
	}

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

unique_ptr<AST_Pointer_Dereference> AST::ParsePointerDereferenceExpression()
{
	unique_ptr<Expression> expr = ParseExpression();
	assert(expr->type.pointerLevel > 0, "Can't dereference a non-pointer type", GetCurrentLineNum());

	unique_ptr<AST_Pointer_Dereference> derefExpr = make_unique<AST_Pointer_Dereference>();
	derefExpr->type.pointerLevel = expr->type.pointerLevel - 1;
	derefExpr->baseExpr = std::move(expr);
	return std::move(derefExpr);
}

