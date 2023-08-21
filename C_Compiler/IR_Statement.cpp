#include "IR_Statement.h"
#include <sstream>

using std::stringstream;

string conditionToString(FlagResults condition)
{
	string cs = "";
	switch (condition)
	{
	case IR_EQUALS: cs = "E"; break;
	case IR_NOT_EQUALS: cs = "NE"; break;
	case IR_GREATER: cs = "GT"; break;
	case IR_GREATER_EQUALS: cs = "GE"; break;
	case IR_LESS: cs = "LT"; break;
	case IR_LESS_EQUALS: cs = "LE"; break;
	case IR_FLOAT_GREATER: cs = "A"; break;
	case IR_FLOAT_GREATER_EQUALS: cs = "AE"; break;
	case IR_FLOAT_LESS: cs = "B"; break;
	case IR_FLOAT_LESS_EQUALS: cs = "BE"; break;
	}

	return cs;
}

string varToString(IR_Value v)
{
	stringstream ss;

	string varType = "";
	if (v.type == IR_STRUCT)
	{
		varType = "STRUCT";
	}
	else if (v.type == IR_FLOAT)
	{
		varType = "FLOAT";
	}
	else {
		varType = "INT";
	}

	if (v.valueType == IR_LITERAL)
	{
		ss << "LITERAL (" << v.literalValue + ")";
	}
	else if (v.specialVars == IR_FLAGS)
	{
		ss << "%flags: " << conditionToString(v.flag);
	}
	else if (v.specialVars == IR_RETURN_INT || v.specialVars == IR_RETURN_FLOAT || v.specialVars == IR_RETURN_STACK)
	{
		ss << "%return - " << v.specialVars << " (" << varType << string(v.pointerLevel, '*') << ", " << v.byteSize << ")";
	}
	else {
		ss << "%" << (v.isTempValue ? "tmp" : "v") << v.varIndex << "(" << varType
			<< (v.pointerLevel > 0 ? string(v.pointerLevel, '*') : "") << ", " << v.byteSize << ")";
	}

	return ss.str();
}


string operandToString(IR_Operand operand)
{
	//TODO: add more!!!!
	stringstream ss;

	//set var type and byte size and pointer level based on if it's a derefenced value
	operand.value.byteSize = operand.value.byteSize;
	operand.value.type = operand.value.type;
	operand.value.pointerLevel = operand.value.pointerLevel;
	if (operand.dereference && operand.useMemoryAddress)
	{
		operand.dereference = false;
		operand.useMemoryAddress = false;
	}

	if (operand.globalFloatValue != 0)
	{
		ss << "[var #" << operand.globalFloatValue << "]";
		return ss.str();
	}

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
IR_Operand::IR_Operand(IR_Value value): value(value), baseOffset(0) {}

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
	case IR_STRUCT_COPY:
		assignType = "STRUCT_COPY";
		break;
	case IR_LEA:
		assignType = "LEA";
		break;
	}
	
	stringstream ss;
	ss << "Assign (" << byteSize << ") " << assignType << " " << operandToString(dest) << ", " << operandToString(source);
	return ss.str();
}

IR_Assign::IR_Assign() {}
IR_Assign::IR_Assign(IR_VarType type, IR_AssignType assignType, int byteSize, IR_Operand dest, IR_Operand source): type(type), assignType(assignType), byteSize(byteSize), dest(dest), source(source) {}
IR_StatementType IR_Assign::GetType()
{
	return _IR_ASSIGN;
}

void IR_Assign::ConvertToX64(x64_State& state)
{
	//TODO: Finish
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
void IR_VariableInit::ConvertToX64(x64_State& state)
{
	//TODO: Finish
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
void IR_Label::ConvertToX64(x64_State& state)
{
	//TODO: Finish
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
void IR_ScopeStart::ConvertToX64(x64_State& state)
{
	//TODO: Finish
}

string IR_ScopeEnd::ToString()
{
	return "Scope End";
}
IR_StatementType IR_ScopeEnd::GetType()
{
	return _IR_SCOPE_END;
}
void IR_ScopeEnd::ConvertToX64(x64_State& state)
{
	//TODO: Finish
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
void IR_Jump::ConvertToX64(x64_State& state)
{
	//TODO: Finish
}

IR_Jump::IR_Jump(int labelIdx, FlagResults condition): labelIdx(labelIdx), condition(condition) {}


IR_FunctionCall::IR_FunctionCall(string funcName) : funcName(funcName) {}
string IR_FunctionCall::ToString()
{
	stringstream ss;
	ss << "CALL Function: " << this->funcName;
	return ss.str();
}
IR_StatementType IR_FunctionCall::GetType()
{
	return _IR_FUNCTION_CALL;
}
void IR_FunctionCall::ConvertToX64(x64_State& state)
{
	//TODO: Finish
}

//string IR_RegisterWriteToMemory::ToString()
//{
//	return "WRITE REGISTERS TO MEMORY";
//}
//IR_StatementType IR_RegisterWriteToMemory::GetType()
//{
//	return _IR_REGISTER_WRITE_TO_MEMORY;
//}

string IR_ContinuousMemoryInit::ToString()
{
	stringstream ss;
	ss << "Init Continuous Memory: %" << varIdx << " - " << byteNum << " bytes";
	return ss.str();
}
IR_StatementType IR_ContinuousMemoryInit::GetType()
{
	return _IR_CONTINUOUS_MEMORY_INIT;
}
void IR_ContinuousMemoryInit::ConvertToX64(x64_State& state)
{
	//TODO: Finish
}

string IR_FunctionArgAssign::ToString()
{
	stringstream ss;
	string type;
	if (this->argType == IR_INT_ARG)
	{
		type = "INT";
	}
	else if (this->argType == IR_FLOAT_ARG)
	{
		type = "FLOAT";
	}
	else {
		type = "STACK";
	}
	ss << "Arg " << type << " #" << this->argIdx << "(size: " << this->byteSize << ", offset: " << this->stackArgOffset << ")" << ": " << operandToString(this->value);
	return ss.str();
}
IR_StatementType IR_FunctionArgAssign::GetType()
{
	return _IR_FUNCTION_ARG_ASSIGN;
}
void IR_FunctionArgAssign::ConvertToX64(x64_State& state)
{
	//TODO: Finish
}

IR_FunctionArgAssign::IR_FunctionArgAssign() {}
IR_FunctionArgAssign::IR_FunctionArgAssign(int argIdx, IR_Operand value, IR_FunctionArgType argType, int byteSize, int stackArgOffset): 
	argIdx(argIdx), value(value), argType(argType), byteSize(byteSize), stackArgOffset(stackArgOffset) {}


//string IR_VariableReload::ToString()
//{
//	return "VARIABLE RELOAD";
//}
//IR_StatementType IR_VariableReload::GetType()
//{
//	return _IR_VARIABLE_RELOAD;
//}

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
void IR_Compare::ConvertToX64(x64_State& state)
{
	//TODO: Finish
}

string IR_NOP::ToString()
{
	return "NOP";
}

IR_StatementType IR_NOP::GetType()
{
	return _IR_NOP;
}
void IR_NOP::ConvertToX64(x64_State& state)
{
	//TODO: Finish
}

string IR_LoopStart::ToString()
{
	return "LOOP START";
}
IR_StatementType IR_LoopStart::GetType()
{
	return _IR_LOOP_START;
}
void IR_LoopStart::ConvertToX64(x64_State& state)
{
	//TODO: Finish
}

string IR_LoopEnd::ToString()
{
	return "LOOP END";
}
IR_StatementType IR_LoopEnd::GetType()
{
	return _IR_LOOP_END;
}
void IR_LoopEnd::ConvertToX64(x64_State& state)
{
	//TODO: Finish
}

string IR_FunctionLabel::ToString()
{
	stringstream ss;

	ss << "Function: " << this->functionName << "(";

	for (const IR_Value& value : this->args)
	{
		ss << varToString(value) << ", "; //note: this will add extra unnecessary comma (not important b/c this is just for debugging)
	}

	ss << ") -> ";
	ss << varToString(this->returnValue) << "(" << this->returnValueByteSize << ")";
	return ss.str();
}

IR_StatementType IR_FunctionLabel::GetType()
{
	return _IR_FUNCTION_LABEL;
}
void IR_FunctionLabel::ConvertToX64(x64_State& state)
{
	//TODO: Finish
}

IR_FunctionLabel::IR_FunctionLabel(string functionName) : functionName(functionName) {}
IR_FunctionLabel::IR_FunctionLabel() {}

string IR_Return::ToString()
{
	return "RET";
}

IR_StatementType IR_Return::GetType()
{
	return _IR_RETURN;
}
void IR_Return::ConvertToX64(x64_State& state)
{
	//TODO: Finish
}

string IR_FunctionStart::ToString()
{
	return "FUNCTION START";
}
IR_StatementType IR_FunctionStart::GetType()
{
	return _IR_FUNCTION_START;
}
void IR_FunctionStart::ConvertToX64(x64_State& state)
{
	//TODO: Finish
	state.registerAllocator.memoryVariableMapping.memoryOffsetMapping.clear();
}

string IR_FunctionEnd::ToString()
{
	return "FUNCTION END";
}
IR_StatementType IR_FunctionEnd::GetType()
{
	return _IR_FUNCTION_END;
}
void IR_FunctionEnd::ConvertToX64(x64_State& state)
{
	//TODO: Finish
}