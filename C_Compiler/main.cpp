#include "Parser.h"
#include "IR_CodeGen.h"
#include "x64_CodeGen.h"

int main()
{

	const string input = Utils::ReadFile("test_code.txt");

	auto tokens = Lexer::SplitStringByToken(input);
	std::cout << input << "\n\n";
	/*for (auto& s : tokens) {
		std::cout << s.type << ": " << Lexer::GetNameFromEnum(s.type) << ": " << s.value << " - line: " << s.lineNumber << " - token: " << s.tokenNumber << "\n";
	}*/

	//Convert tokens to AST
	AST ast(tokens);
	ast.ParseProgram();
	ast.group->PrintStatementAST();

	//Convert AST to IR
	IR_CodeGen irCode(ast.group);
	irCode.ConvertToIR();
	irCode.PrintIR();

	std::cout << "-------------\n\n\n";

	//Optimize IR
	irCode.Optimize();
	irCode.PrintIR();

	std::cout << "-------------\n\n\n";

	//Convert IR to x64
	x64_CodeGen x64CodeGen(irCode.irState);

	x64CodeGen.GenerateCode();
	string output = x64CodeGen.CodeToString();
	
	//Write output to file
}
