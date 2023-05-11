// C_Compiler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "Parser.h"

using std::string;
using std::vector;
using std::unordered_map;

using namespace Parser;
//
//struct Statement {};
//struct Expression {};
//struct BinOperator {};
//
////enum LValueType {
////	INT,
////	FLOAT
////};
//
//struct LValue {
//	LValueType type;
//};
//struct AssignmentOperator {};
//
//struct AST_BinOp {
//	BinOperator op;
//	Expression left;
//	Expression right;
//	const static int precedence;
//};
//
//struct AST_BinOp_Add : AST_BinOp {
//	const static int precedence = 1;
//};
//struct AST_Bin_Op_Mult : AST_BinOp {
//	const static int precendence = 2;
//};
//
//struct AST_Assignment {
//	AssignmentOperator op;
//	LValue lvalue;
//	Expression rvalue;
//};
//
//struct AST_If_Then : Statement {
//	Expression Condition;
//	Statement ifStatement;
//	Statement elseStatement;
//};
//
//struct StatementGroup : Statement {
//	vector<Statement> statements;
//};




class AST
{
public:
	vector<Token> tokens;
	//StatementGroup group;

	size_t currentIndex = 0;
	//size_t scopeLevel = 0;


	//Expression ParseCondition()
	//{

	//}

	//AST_If_Then ParseIfThen()
	//{
	//	AST_If_Then ifThen;
	//	ifThen.Condition = ParseCondition();

	//	++scopeLevel;
	//	ifThen.ifStatement = ParseStatement();
	//	--scopeLevel;

	//	if (tokens.at(currentIndex).type == TokenType::ELSE)
	//	{
	//		++scopeLevel;
	//		ifThen.elseStatement = ParseStatement();
	//		--scopeLevel;
	//	}

	//}

	//Statement ParseStatement()
	//{
	//	if (tokens.at(currentIndex).type == TokenType::IF) {
	//		group.statements.push_back(ParseIfThen());
	//	}
	//}

};


int main()
{
	const string input = "z=name.q; int y = x + 1; if(x == 2) {y++; }";
	auto x = SplitStringByToken(input);
	std::cout << input << "\n\n";
	for (auto& s : x) {
		std::cout << Parser::GetNameFromEnum(s.type) << ": " << s.value << "\n";
	}

}


//have state object (representing current scope, defined variables / structs, etc...
//iterate through vector of strings
 

