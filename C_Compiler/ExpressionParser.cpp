#include "Parser.h"


unique_ptr<Expression> AST::ParseExpression()
{
	unique_ptr<Expression> firstExpr;
	if (tokens.at(currentIndex).type == TokenType::OPEN_PAR)
	{
		++currentIndex;
		if (Lexer::IsTypeToken(GetCurrentToken().type)) //parse type casting expression
		{
			firstExpr = ParseTypeCastExpression();
		}
		else
		{
			firstExpr = ParseParentheticalExpression();
		}

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
	else if (Lexer::IsAssignmentOp(token.type))
	{
		return ParseAssignmentExpression(ConvertExpressionToLValue(std::move(firstExpr)));
	}
	else
	{
		return firstExpr;
	}
}

template<class T>
unique_ptr<T> AST::ConvertToSubexpression(unique_ptr<Expression>&& expr)
{
	T* tmp = dynamic_cast<T*>(expr.get());
	unique_ptr<T> subExpr;
	if (tmp != nullptr)
	{
		expr.release();
		subExpr.reset(tmp);
	}
	else
	{
		throwError("Error converting expression to subexpression type", GetCurrentLineNum());
	}

	return subExpr;
}

unique_ptr<LValueExpression> AST::ConvertExpressionToLValue(unique_ptr<Expression>&& expr)
{
	assert(expr->isLValue, "Can only use unary assignment operator on LValue", GetCurrentLineNum());

	return ConvertToSubexpression<LValueExpression>(std::move(expr));
}

unique_ptr<LValueExpression> AST::ParseLValueExpression()
{
	unique_ptr<Expression> expr = AST::ParseNonBinaryExpression();
	return ConvertExpressionToLValue(std::move(expr));
}

unique_ptr<Expression> AST::ParseUnaryExpression()
{
	Token& token = GetCurrentToken();

	++currentIndex;

	if (token.type == TokenType::NOT)
	{
		unique_ptr<AST_Not_Expression> notExpr = make_unique<AST_Not_Expression>();
		notExpr->expr = ParseNonBinaryExpression();

		assert(notExpr->expr->type.lValueType == LValueType::INT || notExpr->expr->type.pointerLevel > 0, "Invalid use of ! operator", token.lineNumber);
		notExpr->type = VariableType(LValueType::INT, "", 0);

		return notExpr;
	}
	else if (token.type == TokenType::MINUS) {
		unique_ptr<Expression> expr = ParseNonBinaryExpression();

		//special case for parsing negative int/float literals
		if(expr->GetExpressionType() == ExpressionType::_Literal_Expression)
		{
			unique_ptr<AST_Literal_Expression> litExpr = ConvertToSubexpression<AST_Literal_Expression>(std::move(expr));
			assert(litExpr->value[0] != '-', "Double negative not allowed", token.lineNumber);
			litExpr->value = "-" + litExpr->value;

			return std::move(litExpr);
		}

		unique_ptr<AST_Negative_Expression> negativeExpr = make_unique<AST_Negative_Expression>();
		negativeExpr->expr = std::move(expr);

		assert((negativeExpr->expr->type.lValueType == LValueType::INT || negativeExpr->expr->type.lValueType == LValueType::FLOAT) 
			&& negativeExpr->expr->type.pointerLevel == 0,
			"Can't take negative of struct or pointer value", token.lineNumber);
		negativeExpr->type = negativeExpr->expr->type;

		return negativeExpr;

	}
	else //TokenType::ADDRESSS_OF
	{
		unique_ptr<AST_Address_Expression> addrExpr = make_unique<AST_Address_Expression>();
		addrExpr->expr = ParseNonBinaryExpression();

		assert(addrExpr->expr->isLValue, "Can only get reference to LValue type", token.lineNumber);
		addrExpr->type = VariableType(addrExpr->expr->type.lValueType, addrExpr->expr->type.structName, addrExpr->expr->type.pointerLevel + 1);

		return addrExpr;
	}


}

unique_ptr<AST_Pointer_Offset> AST::ParseArrayIndexExpression(unique_ptr<Expression>&& prev)
{
	assert(prev->type.pointerLevel > 0, "Can only index a pointer type variable", GetCurrentLineNum());

	++currentIndex;
	unique_ptr<Expression> indexExpr = ParseNonBinaryExpression();
	assert(indexExpr->type.lValueType == LValueType::INT && indexExpr->type.pointerLevel == 0, "Array index must be integer value", GetCurrentLineNum());

	unique_ptr<AST_Pointer_Offset> arrayIndexExpr = make_unique<AST_Pointer_Offset>();
	arrayIndexExpr->expr = std::move(prev);
	arrayIndexExpr->index = std::move(indexExpr);

	arrayIndexExpr->type = arrayIndexExpr->expr->type;

	assert(GetCurrentToken().type == TokenType::CLOSE_BRACKET, "Array index must end with close bracket", GetCurrentLineNum());

	++currentIndex;

	return arrayIndexExpr;
}

VariableType AST::ParseVariableType()
{
	VariableType type;
	Token& token = GetCurrentToken();

	if (token.type == TokenType::STRUCT)
	{
		type.lValueType = LValueType::STRUCT;
		++currentIndex;

		Token& structNameToken = GetCurrentToken();
		assert(structNameToken.type == TokenType::NAME, "Must have struct name in variable type", structNameToken.lineNumber);
		type.structName = structNameToken.value;
	}
	else
	{
		Token& typeNameToken = GetCurrentToken();
		assert(typeNameToken.type == TokenType::TYPE || typeNameToken.type == TokenType::VOID, "Must have valid variable type", typeNameToken.lineNumber);

		if (typeNameToken.type == TokenType::TYPE)
		{
			type.lValueType = typeNameToken.value == "int" ? LValueType::INT : LValueType::FLOAT;
		}
		else if (typeNameToken.type == TokenType::VOID)
		{
			type.lValueType = LValueType::VOID;
		}

	}

	++currentIndex;
	type.pointerLevel = GetConsecutiveTokenNumber(TokenType::STAR);

	assert(type.pointerLevel > 0 || type.lValueType != LValueType::VOID, "Non-pointer void type does not exist", GetCurrentLineNum());

	return type;
}

unique_ptr<AST_Type_Cast_Expression> AST::ParseTypeCastExpression()
{
	VariableType toType = ParseVariableType();
	assert(GetCurrentToken().type == TokenType::CLOSE_PAR, "Type cast must end with close parentheses", GetCurrentLineNum());
	++currentIndex;

	unique_ptr<AST_Type_Cast_Expression> typeCastExpr = make_unique<AST_Type_Cast_Expression>();
	typeCastExpr->to = toType;
	typeCastExpr->expr = ParseNonBinaryExpression();
	typeCastExpr->from = typeCastExpr->expr->type;

	typeCastExpr->type = std::move(toType);

	assert(typeCastExpr->IsValidTypeCast(), "Invalid type cast", GetCurrentLineNum());

	return typeCastExpr;

}

unique_ptr<Expression> AST::ParseNonBinaryExpression(unique_ptr<Expression> prev)
{
	unique_ptr<Expression> expr;
	Token& token = tokens.at(currentIndex);
	Token& nextToken = tokens.at(currentIndex + 1);

	//end of non-binary expression
	
	if (token.type == TokenType::OPEN_PAR)
	{
		++currentIndex;
		if (Lexer::IsTypeToken(nextToken.type)) //parse type casting expression
		{
			assert(!prev, "Invalid type cast expression syntax (must be start of expression)", nextToken.lineNumber);
			return ParseTypeCastExpression();
		}
		else //parse parenthetical expression
		{
			return ParseNonBinaryExpression(ParseParentheticalExpression());

		}
	}
	else if (Lexer::IsUnaryOp(token.type)) // !, -, &
	{
		assert(!prev, "Invalid unary op syntax", token.lineNumber);
		return ParseUnaryExpression();
	}
	else if (Lexer::IsUnaryAssignmentOp(token.type)) //prefix unary op
	{
		unique_ptr<AST_Unary_Assignment_Expression> unaryExpr = make_unique<AST_Unary_Assignment_Expression>();
		unaryExpr->opType = token.type;
		++currentIndex;

		if (!prev)
		{
			unaryExpr->isPrefix = true;
			unaryExpr->expr = ParseLValueExpression();
		}
		else
		{
			unaryExpr->isPrefix = false;
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
			bool foundFunction = scopeStack.TryFindFunction(token.value, f.def);
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
	else if (token.type == TokenType::ARROW) //dereference struct pointer
	{
		unique_ptr<AST_Pointer_Dereference> ptrDerefExpr = make_unique<AST_Pointer_Dereference>();
		ptrDerefExpr->baseExpr = std::move(prev);
		return ParseStructVariableAccess(std::move(ptrDerefExpr));
	}
	else if (token.type == TokenType::OPEN_BRACKET) //accessing array element
	{
		unique_ptr<AST_Pointer_Offset> expr = ParseArrayIndexExpression(std::move(prev));
		return ParseNonBinaryExpression(make_unique<AST_Pointer_Dereference>(std::move(expr)));
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
	else if (Lexer::IsNonBinaryExpressionTerminalToken(token.type))
	{
		assert(prev != nullptr, "Expression is empty", token.lineNumber);
		return prev;
	}
	else if (token.type == TokenType::CLOSE_PAR)
	{
		return prev;
	}
	else
	{
		throwError("Invalid sytnax", GetCurrentLineNum());
	}

}

void AST::PerformImplicitTypeCastBinaryOp(unique_ptr<AST_BinOp>& binOpExpr)
{
	if (binOpExpr->left->type.lValueType == LValueType::INT && binOpExpr->right->type.lValueType == LValueType::FLOAT 
		&& binOpExpr->left->type.pointerLevel == 0 && binOpExpr->right->type.pointerLevel == 0)
	{
		binOpExpr->left = make_unique<AST_Type_Cast_Expression>(std::move(binOpExpr->left), VariableType(LValueType::INT, "", 0), VariableType(LValueType::FLOAT, "", 0));
	}
	else if (binOpExpr->left->type.lValueType == LValueType::FLOAT && binOpExpr->right->type.lValueType == LValueType::INT
		&& binOpExpr->left->type.pointerLevel == 0 && binOpExpr->right->type.pointerLevel == 0)
	{
		binOpExpr->right = make_unique<AST_Type_Cast_Expression>(std::move(binOpExpr->right), VariableType(LValueType::INT, "", 0), VariableType(LValueType::FLOAT, "", 0));
	}
}

unique_ptr<AST_Pointer_Offset> AST::ParsePointerArithmetic(unique_ptr<AST_BinOp>&& binOpExpr)
{
	unique_ptr<AST_Pointer_Offset> ptrArithExpr = make_unique<AST_Pointer_Offset>();
	ptrArithExpr->expr = std::move(binOpExpr->left);

	if (binOpExpr->op == BinOpType::ADD)
	{
		ptrArithExpr->index = std::move(binOpExpr->right);
	}
	else
	{
		ptrArithExpr->index = make_unique<AST_Negative_Expression>(std::move(binOpExpr->right));
	}

	ptrArithExpr->type = ptrArithExpr->expr->type;
	return ptrArithExpr;
}


unique_ptr<Expression> AST::ParseBinaryExpression(unique_ptr<Expression> firstExpr)
{
	const BinOp& op = ExpressionUtils::BinOpTokenDictionary.at(tokens.at(currentIndex).type);
	++currentIndex;

	unique_ptr<Expression> secondExpr = ParseNonBinaryExpression();

	//TODO: Check to make sure types of first and second expression match (unless first type is pointer and second is int)
	Token& currentToken = tokens.at(currentIndex);

	unique_ptr<AST_BinOp> binOpExpr = make_unique<AST_BinOp>();
	binOpExpr->left = std::move(firstExpr);
	binOpExpr->op = op.type;

	binOpExpr->type.lValueType = IsBooleanOperation(op.type) ? LValueType::INT : binOpExpr->left->type.lValueType; //figure out whether to treat INT and BOOL separately

	if (Lexer::IsBinOp(currentToken.type))
	{
		const BinOp& secondOp = ExpressionUtils::BinOpTokenDictionary.at(currentToken.type);
		if (op.precedence >= secondOp.precedence)
		{
			binOpExpr->right = std::move(secondExpr);

			PerformImplicitTypeCastBinaryOp(binOpExpr);

			//TODO: Add casting from int to float if one value is int and the other is float; ALSO, do BOOL conversions to INT/FLOAT, etc...
			assert((IsNumericType(binOpExpr->left->type.lValueType) && IsNumericType(binOpExpr->right->type.lValueType))
				|| (binOpExpr->left->type.pointerLevel > 0 && binOpExpr->right->type.lValueType == LValueType::INT && binOpExpr->right->type.pointerLevel == 0
					&& binOpExpr->op == BinOpType::ADD || binOpExpr->op == BinOpType::SUBTRACT),
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

	PerformImplicitTypeCastBinaryOp(binOpExpr);

	assert(binOpExpr->left->type == binOpExpr->right->type
		|| (binOpExpr->left->type.pointerLevel > 0 && binOpExpr->right->type.lValueType == LValueType::INT && binOpExpr->right->type.pointerLevel == 0 
			&& binOpExpr->op == BinOpType::ADD || binOpExpr->op == BinOpType::SUBTRACT),
		"Type mismatch in binary operation", currentToken.lineNumber);

	if (binOpExpr->left->type.pointerLevel > 0 && binOpExpr->right->type.lValueType == LValueType::INT
		&& binOpExpr->right->type.pointerLevel == 0 && (binOpExpr->op == BinOpType::ADD || binOpExpr->op == BinOpType::SUBTRACT))
	{
		return ParsePointerArithmetic(std::move(binOpExpr));
	}

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
	unique_ptr<Expression> expr = ParseNonBinaryExpression();
	assert(expr->type.pointerLevel > 0, "Can't dereference a non-pointer type", GetCurrentLineNum());

	unique_ptr<AST_Pointer_Dereference> derefExpr = make_unique<AST_Pointer_Dereference>();
	derefExpr->type = VariableType(expr->type.lValueType, expr->type.structName, expr->type.pointerLevel - 1);
	derefExpr->baseExpr = std::move(expr);
	return std::move(derefExpr);
}

