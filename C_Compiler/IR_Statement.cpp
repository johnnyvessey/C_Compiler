#include "IR_Statement.h"


string IR_Assign::ToString()
{
	return "";
}

IR_Assign::IR_Assign() {}
IR_Assign::IR_Assign(IR_VarType type, IR_AssignType assignType, IR_Value dest, IR_Value source): type(type), assignType(assignType), dest(dest), source(source) {}


string IR_Label::ToString()
{
	return "";
}

string IR_ScopeStart::ToString()
{
	return "";
}

string IR_ScopeEnd::ToString()
{
	return "";
}

string IR_Branch::ToString()
{
	return "";
}

//string IR_ALUBinOp::ToString()
//{
//	return "";
//}

string IR_FunctionCall::ToString()
{
	return "";
}

string IR_TypeCast::ToString()
{
	return "";
}

string IR_VariableReload::ToString()
{
	return "";
}

