#include "Parser.h"

int main()
{

	const string input = Utils::ReadFile("test_code.txt");

	auto tokens = Lexer::SplitStringByToken(input);
	std::cout << input << "\n\n";
	/*for (auto& s : tokens) {
		std::cout << s.type << ": " << Lexer::GetNameFromEnum(s.type) << ": " << s.value << " - line: " << s.lineNumber << " - token: " << s.tokenNumber << "\n";
	}*/

	AST ast(tokens);
	ast.ParseProgram();
	ast.group->PrintStatementAST();
}
