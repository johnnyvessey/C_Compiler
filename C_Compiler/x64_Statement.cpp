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
		//TODO: figure this out
	}
	else if (this->reg.reg != _NONE)
	{
		if (this->dereference)
		{
			ss << "[";
		}

		ss << RegisterString::registerStringMapping.at((int)this->reg.reg);

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
				ss << this->regOffsetMultiplier << " * ";
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
	case IR_ALWAYS: return ""; break;
	case IR_EQUALS: return "E"; break;
	case IR_FLOAT_GREATER: return "A"; break;
	case IR_FLOAT_GREATER_EQUALS: return "AE"; break;
	case IR_FLOAT_LESS: return "B"; break;
	case IR_FLOAT_LESS_EQUALS: return "BE"; break;
	case IR_GREATER: return "G"; break;
	case IR_GREATER_EQUALS: return "GE"; break;
	case IR_LESS: return "L"; break;
	case IR_LESS_EQUALS: return "LE"; break;
	case IR_NOT_EQUALS: return "NE"; break;
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
	//TODO: finish this
	stringstream ss;
	
	string doubleOpEnd = this->firstOperand.ToString() + ", " + this->secondOperand.ToString();

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
	case x64_GLOBAL_VARIABLE:
	{
		//TODO: figure this out!
		break;
	}
	//instruction cases
	case x64_MOV:
	{
		ss << "MOV " << doubleOpEnd;
		break;
	}
	case x64_MOVS:
	{
		ss << "MOVS " << doubleOpEnd;
		break;
	}
	case x64_ADD:
	{
		ss << "ADD " << doubleOpEnd;
		break;
	}
	case x64_ADDS:
	{
		ss << "ADDS " << doubleOpEnd;
		break;
	}
	case x64_SUB:
	{
		ss << "SUB " << doubleOpEnd;
		break;
	}
	case x64_SUBS:
	{
		ss << "SUBS " << doubleOpEnd;
		break;
	}
	case x64_IMUL:
	{
		ss << "IMUL " << doubleOpEnd;
		break;
	}
	case x64_MULS:
	{
		ss << "MULS " << doubleOpEnd;
		break;
	}
	case x64_IDIV:
	{
		//TODO: figure this out
		break;
	}
	case x64_DIVS:
	{
		ss << "DIVS " << doubleOpEnd;
		break;
	}
	case x64_CMP:
	{
		ss << "CMP " << doubleOpEnd;
		break;
	}
	case x64_CALL:
	{
		ss << "CALL " << this->name;
		break;
	}
	case x64_JMP:
	{
		ss << "J";
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
		ss << "CQO";
		break;
	}
	case x64_LEA:
	{
		ss << "LEA " << doubleOpEnd;
		break;
	}
	case x64_CVTSI2SD:
	{
		ss << "CVTSI2SD " << doubleOpEnd;
		break;
	}
	case x64_CVTTSD2SI:
	{
		ss << "CVTTSD2SI " << doubleOpEnd;
		break;
	}
	case x64_PUSH:
	{
		ss << "PUSH " << this->firstOperand.ToString();
		break;
	}
	case x64_POP:
	{
		ss << "POP " << this->firstOperand.ToString();
		break;
	}
	case x64_RET:
	{
		ss << "RET";
		break;
	}
	case x64_SET:
	{
		//TODO: figure this out!!
		break;
	}
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

StatementAsm::StatementAsm() : type(x64_NONE) {}
StatementAsm::StatementAsm(StatementAsmType type): type(type) {}
StatementAsm::StatementAsm(StatementAsmType type, string name) : type(type), name(name) {}