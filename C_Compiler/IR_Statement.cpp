#include "IR_Statement.h"
#include <sstream>

using std::stringstream;

string conditionToString(IR_FlagResults condition)
{
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

	return cs;
}

string varToString(IR_Value v)
{
	stringstream ss;
	if (v.valueType == IR_LITERAL)
	{
		ss << "LITERAL (" << v.literalValue + ")";
	}
	else if (v.specialVars == IR_FLAGS)
	{
		ss << "%flags: " << conditionToString(v.flag);
	}
	else if (v.specialVars == IR_RETURN)
	{
		ss << "%return (" << (v.type == IR_INT ? "INT" : "FLOAT") << ", " << v.byteSize << ")";
	}
	else {
		ss << "%" << (v.isTempValue ? "tmp" : "v") << v.varIndex << "(" << (v.type == IR_INT ? "Int" : "Float")
			<< (v.pointerLevel > 0 ? string(v.pointerLevel, '*') : "") << ", " << v.byteSize << ")";
	}

	return ss.str();
}


string operandToString(IR_Operand operand)
{
	//TODO: add more!!!!
	stringstream ss;

	//set var type and byte size and pointer level based on if it's a derefenced value
	operand.value.byteSize = operand.GetByteSize();
	operand.value.type = operand.GetVarType();
	operand.value.pointerLevel = operand.GetPointerLevel();

	if (operand.dereference)
	{
		ss << "[" << varToString(operand.value);
		if (operand.baseOffset != 0)
		{
			ss << " + " << operand.baseOffset;
		}
		if (operand.memoryOffsetMultiplier != 0)
		{
			if (operand.memoryOffsetMultiplier == 1)
			{
				ss << " + " << varToString(operand.memoryOffset);
			}
			else {
				ss << " + " << operand.memoryOffsetMultiplier << " * " << varToString(operand.memoryOffset);
			}
		}

		ss << "]";
	}
	else if (operand.useMemoryAddress)
	{
		ss << "Address(" << varToString(operand.value) << ")";
	}
	else {
		ss << varToString(operand.value);
	}
	return ss.str();
}

IR_Value::IR_Value() {}
IR_Value::IR_Value(IR_VarType type, IR_ValueType valueType, int byteSize, int varIndex, bool isTempValue, string literalValue, IR_SpecialVars specialVars) :
	type(type), valueType(valueType), byteSize(byteSize), varIndex(varIndex), isTempValue(isTempValue), literalValue(literalValue), specialVars(specialVars),
	baseType(type) {} 

IR_Value::IR_Value(IR_VarType type, IR_ValueType valueType, int byteSize, int varIndex, bool isTempValue, string literalValue, IR_SpecialVars specialVars, 
	int pointerLevel, IR_VarType baseType) :
	type(type), valueType(valueType), byteSize(byteSize), varIndex(varIndex), isTempValue(isTempValue), literalValue(literalValue), specialVars(specialVars),
	pointerLevel(pointerLevel), baseType(baseType) {}

IR_Operand::IR_Operand() {}
IR_Operand::IR_Operand(IR_Value value): value(value) {}

int IR_Operand::GetPointerLevel()
{
	int pointerLevelOffset = dereference ? -1 : 0;
	int refOffset = useMemoryAddress ? 1 : 0;

	return value.pointerLevel + pointerLevelOffset + refOffset;
}
IR_VarType IR_Operand::GetVarType()
{
	return GetPointerLevel() > 0 ? IR_VarType::IR_INT : this->value.baseType;
}
int IR_Operand::GetByteSize()
{
	return GetPointerLevel() > 0 ? POINTER_SIZE : 4;
}

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
	case IR_FLAG_CONVERT:
		assignType = "FLAG";
		break;
	}
	
	stringstream ss;
	ss << "Assign " << assignType << " " << operandToString(dest) << ", " << operandToString(source);
	return ss.str();
}

IR_Assign::IR_Assign() {}
IR_Assign::IR_Assign(IR_VarType type, IR_AssignType assignType, int byteSize, IR_Operand dest, IR_Operand source): type(type), assignType(assignType), byteSize(byteSize), dest(dest), source(source) {}
IR_StatementType IR_Assign::GetType()
{
	return _IR_ASSIGN;
}

IR_VariableInit::IR_VariableInit() {}
IR_VariableInit::IR_VariableInit(IR_Value value) : dest(value) {}
string IR_VariableInit::ToString()
{
	return "INIT " + varToString(dest);
}

IR_StatementType IR_VariableInit::GetType()
{
	return _IR_VARIABLE_INIT;
}

string IR_Label::ToString()
{
	stringstream ss;
	ss << "Label #" << label << ":";
	return ss.str();
}
IR_StatementType IR_Label::GetType()
{
	return _IR_LABEL;
}
IR_Label::IR_Label(int label) : label(label) {}


string IR_ScopeStart::ToString()
{
	return "Scope Start";
}
IR_StatementType IR_ScopeStart::GetType()
{
	return _IR_SCOPE_START;
}

string IR_ScopeEnd::ToString()
{
	return "Scope End";
}
IR_StatementType IR_ScopeEnd::GetType()
{
	return _IR_SCOPE_END;
}

string IR_Jump::ToString()
{
	stringstream ss;


	ss << "J" << conditionToString(condition) << " #" << labelIdx;
	return ss.str();
}
IR_StatementType IR_Jump::GetType()
{
	return _IR_JUMP;
}

IR_Jump::IR_Jump(int labelIdx, IR_FlagResults condition): labelIdx(labelIdx), condition(condition) {}


string IR_FunctionCall::ToString()
{
	return "";
}
IR_StatementType IR_FunctionCall::GetType()
{
	return _IR_FUNCTION_CALL;
}

string IR_RegisterWriteToMemory::ToString()
{
	return "WRITE REGISTERS TO MEMORY";
}
IR_StatementType IR_RegisterWriteToMemory::GetType()
{
	return _IR_REGISTER_WRITE_TO_MEMORY;
}

string IR_StructInit::ToString()
{
	return "";
}
IR_StatementType IR_StructInit::GetType()
{
	return _IR_STRUCT_INIT;
}

string IR_FunctionArgAssign::ToString()
{
	return "";
}
IR_StatementType IR_FunctionArgAssign::GetType()
{
	return _IR_FUNCTION_ARG_ASSIGN;
}

IR_FunctionArgAssign::IR_FunctionArgAssign() {}
IR_FunctionArgAssign::IR_FunctionArgAssign(int argIdx, IR_Operand value): argIdx(argIdx), value(value) {}


string IR_VariableReload::ToString()
{
	return "VARIABLE RELOAD";
}
IR_StatementType IR_VariableReload::GetType()
{
	return _IR_VARIABLE_RELOAD;
}

IR_Compare::IR_Compare() {}
IR_Compare::IR_Compare(IR_Operand op1, IR_Operand op2): op1(op1), op2(op2) {}

string IR_Compare::ToString()
{
	stringstream ss;

	ss << "Compare: " << operandToString(op1) << ", " << operandToString(op2);
	return ss.str();
}
IR_StatementType IR_Compare::GetType()
{
	return _IR_COMPARE;
}

string IR_NOP::ToString()
{
	return "NOP";
}

IR_StatementType IR_NOP::GetType()
{
	return _IR_NOP;
}