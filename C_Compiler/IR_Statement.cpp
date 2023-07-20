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

IR_Operand::IR_Operand() {}
IR_Operand::IR_Operand(IR_Value value): value(value) {}

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
	ss << "Assign " << assignType << " " << varToString(dest.value) << ", " << varToString(source.value);
	return ss.str();
}

IR_Assign::IR_Assign() {}
IR_Assign::IR_Assign(IR_VarType type, IR_AssignType assignType, IR_Operand dest, IR_Operand source): type(type), assignType(assignType), dest(dest), source(source) {}

IR_VariableInit::IR_VariableInit() {}
IR_VariableInit::IR_VariableInit(IR_Value value) : dest(value) {}
string IR_VariableInit::ToString()
{
	return "INIT " + varToString(dest);
}

string IR_Label::ToString()
{
	stringstream ss;
	ss << "Label " << label << ":";
	return ss.str();
}
IR_Label::IR_Label(int label) : label(label) {}


string IR_ScopeStart::ToString()
{
	return "Scope Start";
}

string IR_ScopeEnd::ToString()
{
	return "Scope End";
}

string IR_Jump::ToString()
{
	stringstream ss;

	string cs = "";
	switch (condition)
	{
	case IR_EQUALS: cs = "E"; break;
	case IR_NOT_EQUALS: cs = "NE"; break;
	case IR_GREATER: cs = "GT"; break;
	case IR_GREATER_EQUALS: cs = "GTE"; break;
	case IR_LESS: cs = "LT"; break;
	case IR_LESS_EQUALS: cs = "LTE"; break;
	}

	ss << "J" << cs;
	return ss.str();
}

IR_Jump::IR_Jump(int labelIdx, IR_FlagResults condition): labelIdx(labelIdx), condition(condition) {}

//string IR_ALUBinOp::ToString()
//{
//	return "";
//}

string IR_FunctionCall::ToString()
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

string IR_FunctionArgAssign::ToString()
{
	return "";
}

IR_FunctionArgAssign::IR_FunctionArgAssign() {}
IR_FunctionArgAssign::IR_FunctionArgAssign(int argIdx, IR_Operand value): argIdx(argIdx), value(value) {}


//string IR_TypeCast::ToString()
//{
//	return "";
//}

string IR_VariableReload::ToString()
{
	return "";
}


IR_Compare::IR_Compare() {}
IR_Compare::IR_Compare(IR_Operand op1, IR_Operand op2): op1(op1), op2(op2) {}

string IR_Compare::ToString()
{
	return "";
}