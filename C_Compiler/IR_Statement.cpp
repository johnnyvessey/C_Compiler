#include "IR_Statement.h"

IR_Value::IR_Value() {}
IR_Value::IR_Value(IR_VarType type, IR_ValueType valueType, int byteSize, size_t varIndex, bool isTempValue, string literalValue) :
	type(type), valueType(valueType), byteSize(byteSize), varIndex(varIndex), isTempValue(isTempValue), literalValue(literalValue) {}

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

string IR_FunctionArgAssign::ToString()
{
	return "";
}

string IR_RegisterWriteToMemory::ToString()
{
	return "";
}

IR_FunctionArgAssign::IR_FunctionArgAssign() {}
IR_FunctionArgAssign::IR_FunctionArgAssign(size_t argIdx, IR_Value value): argIdx(argIdx), value(value) {}


//string IR_TypeCast::ToString()
//{
//	return "";
//}

string IR_VariableReload::ToString()
{
	return "";
}

