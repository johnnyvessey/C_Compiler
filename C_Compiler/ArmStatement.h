#pragma once
#include <string>
using std::string;

enum ArmStatementType
{

};

enum RegisterType
{
	WORD,
	DWORD,
	FLOAT
};

struct ArmValue {
	string value;
};

struct Register : ArmValue {
	int registerIndex; //figure out if this should be enum //WORD and DWORD types alias each other
	RegisterType regType;
};

struct LiteralValue : ArmValue { //remember only certain numbers can be directly MOV into registers, others have to be loaded/stored

};

struct AddressValue : ArmValue {
	Register reg;
	int offset;
};

struct ArmStatement
{
	ArmStatementType type;
	//Conditional
	//set flags
	//arg 1
	//arg 2

};