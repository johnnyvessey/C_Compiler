#include "x64_Statement.h"


string OperandAsm::ToString() const
{
	stringstream ss;
	if (this->type == ASM_INT_LITERAL)
	{
		ss << this->literalIntValue;
	}
	else if (this->type == ASM_GLOBAL_MEMORY)
	{
		//if xmmword size is used for global memory, only use it for taking negative of float
		if (xmmwordSize)
		{
			ss << "XMMWORD PTR [_negative]";
		}
		else {
			ss << "QWORD PTR [" << this->name << "]";
		}
	}
	else if (this->reg.reg != _NONE)
	{
		if (this->dereference)
		{
			ss << (this->xmmwordSize ? "XMMWORD PTR [" : "QWORD PTR [");
		}

		if (this->reg.size == 1)
		{
			ss << RegisterString::registerStringMapping1byte.at((int)this->reg.reg);
		}
		else {
			ss << RegisterString::registerStringMapping.at((int)this->reg.reg);
		}

		if (this->baseOffset != 0)
		{
			if (this->baseOffset > 0)
			{
				ss << " + " << baseOffset << " ";
			}
			else {
				ss << " - " << -baseOffset << " ";
			}
		}

		if (this->useRegOffset)
		{
			if (this->regOffsetMultiplier != 1)
			{
				ss << " + " << this->regOffsetMultiplier << " * ";
			}
			ss << RegisterString::registerStringMapping.at((int)this->regOffset.reg);
		}

		if (this->dereference)
		{
			ss << "]";
		}
	}


	return ss.str();
}

string FlagsToString(FlagResults flags)
{
	switch (flags)
	{
	case IR_ALWAYS: return "mp"; break;
	case IR_EQUALS: return "e"; break;
	case IR_FLOAT_GREATER: return "a"; break;
	case IR_FLOAT_GREATER_EQUALS: return "ae"; break;
	case IR_FLOAT_LESS: return "b"; break;
	case IR_FLOAT_LESS_EQUALS: return "be"; break;
	case IR_GREATER: return "g"; break;
	case IR_GREATER_EQUALS: return "ge"; break;
	case IR_LESS: return "l"; break;
	case IR_LESS_EQUALS: return "le"; break;
	case IR_NOT_EQUALS: return "ne"; break;
	case IR_NO_FLAGS: return ""; break;
	}

	return "";
}

string LabelToString(int labelIdx)
{
	return "label_" + std::to_string(labelIdx);
}


string StatementAsm::ToString() const
{
	//only print pre and post statements if NOP
	if (this->type == x64_NOP)
	{
		stringstream ss;
		for (const StatementAsm& preStatement : this->preStatements)
		{
			ss << preStatement.ToString() << "\n";
		}
		for (const StatementAsm& postStatement : this->postStatements)
		{
			ss << postStatement.ToString() << "\n";
		}

		string finalString = ss.str();
		if (!finalString.empty())
		{
			finalString.pop_back();
		}
		return finalString;
	}

	stringstream ss;
	
	if (!this->preStatements.empty()) {
		ss << "\n";
	}
	for (const StatementAsm& preStatement : this->preStatements)
	{
		ss << preStatement.ToString() << "\n";
	}

	string doubleOpEnd = this->firstOperand.ToString() + ", " + this->secondOperand.ToString();

	switch (this->type)
	{

		//script format cases
	case x64_DATA_SECTION:
	{
		ss << ".data\n";
		break;
	}
	case x64_CODE_SECTION:
	{
		ss << "\n\n.code";
		break;
	}
	case x64_CODE_END:
	{
		ss << "end";
		break;
	}
	case x64_FUNCTION_PROC:
	{
		ss << "\n" << this->name << " proc ";
		break;
	}
	case x64_FUNCTION_END:
	{
		ss << this->name << " endp";
		break;
	}
	case x64_GLOBAL_VARIABLE:
	{
		ss << this->name;
		break;
	}
	//instruction cases
	case x64_MOV:
	{
		ss << "mov " << doubleOpEnd;
		break;
	}
	case x64_MOVS:
	{
		ss << "movsd " << doubleOpEnd;
		break;
	}
	case x64_ADD:
	{
		ss << "add " << doubleOpEnd;
		break;
	}
	case x64_ADDS:
	{
		ss << "addsd " << doubleOpEnd;
		break;
	}
	case x64_SUB:
	{
		ss << "sub " << doubleOpEnd;
		break;
	}
	case x64_SUBS:
	{
		ss << "subsd " << doubleOpEnd;
		break;
	}
	case x64_IMUL:
	{
		ss << "imul " << doubleOpEnd;
		break;
	}
	case x64_MULS:
	{
		ss << "mulsd " << doubleOpEnd;
		break;
	}
	case x64_IDIV:
	{
		ss << "idiv " << this->firstOperand.ToString();
		break;
	}
	case x64_DIVS:
	{
		ss << "divsd " << doubleOpEnd;
		break;
	}
	case x64_CMP:
	{
		ss << "cmp " << doubleOpEnd;
		break;
	}
	case x64_COMIS: {
		ss << "comisd " << doubleOpEnd;
		break;
	}
	case x64_CALL:
	{
		ss << "call " << this->name;
		break;
	}
	case x64_JMP:
	{
		ss << "j";
		ss << FlagsToString(this->flags);
		ss << " " << name;
		break;
	}
	case x64_LABEL:
	{
		ss << name << ":";
		break;
	}
	case x64_CQO:
	{
		ss << "cqo";
		break;
	}
	case x64_LEA:
	{
		ss << "lea " << doubleOpEnd;
		break;
	}
	case x64_CVTSI2SD:
	{
		ss << "cvtsi2sd " << doubleOpEnd;
		break;
	}
	case x64_CVTTSD2SI:
	{
		ss << "cvttsd2si " << doubleOpEnd;
		break;
	}
	case x64_PUSH:
	{
		ss << "push " << this->firstOperand.ToString();
		break;
	}
	case x64_POP:
	{
		ss << "pop " << this->firstOperand.ToString();
		break;
	}
	case x64_RET:
	{
		ss << "ret";
		break;
	}
	case x64_SET:
	{
		ss << "set" << FlagsToString(this->flags) << " " << this->firstOperand.ToString();
		break;
	}
	case x64_MOVUPS:
	{
		ss << "movups " << doubleOpEnd;
		break;
	}
	case x64_NEG:
	{
		ss << "neg " << this->firstOperand.ToString();
		break;
	}
	case x64_XORPS:
	{
		ss << "xorps " << doubleOpEnd;
		break;
	}
	}


	if (!this->postStatements.empty()) {
		ss << "\n";
	}
	for (const StatementAsm& postStatement : this->postStatements)
	{
		ss << postStatement.ToString() << "\n";
	}

	return ss.str();
}

RegisterAsm::RegisterAsm() {}
RegisterAsm::RegisterAsm(REGISTER reg) : reg(reg) {}


OperandAsm OperandAsm::CreateRSPOffsetOperand(int offset)
{
	OperandAsm op;
	op.dereference = true;
	op.baseOffset = offset;
	op.reg = RegisterAsm(RSP);

	return op;
}

OperandAsm OperandAsm::CreateRBPOffsetOperand(int offset)
{
	OperandAsm op;
	op.dereference = true;
	op.baseOffset = offset;
	op.reg = RegisterAsm(RBP);

	return op;
}

OperandAsm OperandAsm::CreateOffsetOperand(memoryOffset offset)
{
	if (offset.isRsp)
	{
		return OperandAsm::CreateRSPOffsetOperand(offset.offset);
	}
	else {
		return OperandAsm::CreateRBPOffsetOperand(offset.offset);
	}
}


OperandAsm OperandAsm::CreateRegisterOperand(REGISTER reg)
{
	OperandAsm operand;
	operand.reg = RegisterAsm(reg);
	operand.type = ASM_REG;

	return operand;
}

OperandAsm OperandAsm::CreateIntLiteralOperand(int value)
{
	OperandAsm operand;
	operand.literalIntValue = value;
	operand.type = ASM_INT_LITERAL;

	return operand;

}


StatementAsm::StatementAsm() : type(x64_NONE) {}
StatementAsm::StatementAsm(StatementAsmType type): type(type) {}
StatementAsm::StatementAsm(StatementAsmType type, string name) : type(type), name(name) {}

StatementAsm::StatementAsm(StatementAsmType type, OperandAsm firstOperand, OperandAsm secondOperand) : 
	type(type), firstOperand(firstOperand), secondOperand(secondOperand) {}

StatementAsm::StatementAsm(StatementAsmType type, OperandAsm firstOperand) :
	type(type), firstOperand(firstOperand) {}