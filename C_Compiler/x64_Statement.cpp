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
		ss << "qword ptr [_var" << this->name << "]";
	}
	else if (this->reg.reg != _NONE)
	{
		if (this->dereference)
		{
			ss << "qword ptr [";
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
	//TODO: finish this
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
		//TODO: figure this out
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
		//TODO: figure this out!!
		break;
	}
	case x64_NOP: break; //do nothing
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