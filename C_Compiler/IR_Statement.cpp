#include "IR_Statement.h"
#include <sstream>

using std::stringstream;

string varToString(IR_Value& v)
{
	stringstream ss;
	if (v.valueType == IR_LITERAL)
	{
		ss << "LITERAL (" << v.literalValue + ")";
	}
	else {
		ss << "%" << (v.isTempValue ? "tmp" : "v") << v.varIndex << "[" << (v.type == IR_INT ? "Int" : "Float") << ", " << v.byteSize << "]";
	}

	return ss.str();
}

IR_Value::IR_Value() {}
IR_Value::IR_Value(IR_VarType type, IR_ValueType valueType, int byteSize, int varIndex, bool isTempValue, string literalValue) :
	type(type), valueType(valueType), byteSize(byteSize), varIndex(varIndex), isTempValue(isTempValue), literalValue(literalValue) {}

string IR_Assign::ToString()
{
	string assignType;
	switch (this->assignType)
	{
	case IR_ADD:
		assignType = "ADD";
		break;
	case IR_COPY:
		assignType = "COPY";
		break;
	case IR_DIVIDE:
		assignType = "DIV";
		break;
	case IR_FUSED_MULTIPLY_ADD:
		assignType = "FMA";
		break;
	case IR_LEFT_SHIFT:
		assignType = "L_SHIFT";
		break;
	case IR_MULTIPLY:
		assignType = "MUL";
		break;
	case IR_NEGATIVE:
		assignType = "NEG";
		break;
	case IR_RIGHT_SHIFT:
		assignType = "R_SHIFT";
		break;
	case IR_SUBTRACT:
		assignType = "SUB";
		break;
	case IR_TYPE_CAST:
		assignType = "TYPE_CAST";
		break;
	}
	
	stringstream ss;
	ss << "Assign " << assignType << " " << varToString(dest) << ", " << varToString(source);
	return ss.str();
}

IR_Assign::IR_Assign() {}
IR_Assign::IR_Assign(IR_VarType type, IR_AssignType assignType, IR_Value dest, IR_Value source): type(type), assignType(assignType), dest(dest), source(source) {}

IR_VariableInit::IR_VariableInit() {}
IR_VariableInit::IR_VariableInit(IR_Value value) : dest(value) {}
string IR_VariableInit::ToString()
{
	return "INIT " + varToString(dest);
}

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

string IR_StructInit::ToString()
{
	return "";
}
IR_FunctionArgAssign::IR_FunctionArgAssign() {}
IR_FunctionArgAssign::IR_FunctionArgAssign(int argIdx, IR_Value value): argIdx(argIdx), value(value) {}


//string IR_TypeCast::ToString()
//{
//	return "";
//}

string IR_VariableReload::ToString()
{
	return "";
}

