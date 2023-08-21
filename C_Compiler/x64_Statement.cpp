#include "x64_Statement.h"

string StatementAsm::ToString() const
{
	//TODO: implement this
	stringstream ss;
	
	switch (this->type)
	{

	//script format cases
	case x64_DATA_SECTION: 
	{
		ss << ".data";
		break;
	}
	case x64_CODE_SECTION:
	{
		ss << ".code";
		break;
	}
	case x64_CODE_END:
	{
		ss << "end";
		break;
	}
	case x64_FUNCTION_PROC:
	{
		ss << this->name << " proc ";
		break;
	}
	case x64_FUNCTION_END:
	{
		ss << this->name << " endp";
		break;
	}
	}

	//instruction cases

	return ss.str();
}


StatementAsm::StatementAsm(StatementAsmType type): type(type) {}
StatementAsm::StatementAsm(StatementAsmType type, string name) : type(type), name(name) {}