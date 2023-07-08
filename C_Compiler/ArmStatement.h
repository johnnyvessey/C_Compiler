//#pragma once
//#include <string>
//#include <vector>
//using std::string;
//using std::vector;
//
//enum ArmStatementType
//{
//	LITERAL,
//	REGISTER
//};
//
//struct ArmArgument
//{
//	ArmStatementType type;
//};
//
//
//struct Arm_Address : ArmArgument // i.e. [r0]
//{
//	Register reg;
//	//offset?? (could be hard-coded or register)
//	//have pre or post increment (i.e. ! after it) ??
//};
//
//enum RegisterType
//{
//	WORD,
//	DWORD,
//	FLOAT
//};
//
//struct ArmValue {
//	string value;
//};
//
//struct Register : ArmValue {
//	int registerIndex; //figure out if this should be enum //WORD and DWORD types alias each other
//	RegisterType regType;
//};
//
//struct LiteralValue : ArmValue { //remember only certain numbers can be directly MOV into registers, others have to be loaded/stored
//
//};
//
//struct AddressValue : ArmValue {
//	Register reg;
//	int offset;
//};
//
//struct ArmStatement
//{
//	ArmStatementType type;
//	//Conditional
//	//set flags
//	//arg 1
//	//arg 2
//
//};
//
//
//
///*
//* 
//* First pass: infinite registers then apply register allocation algorithm (don't do SSA yet)
//* 
//
//
//*/
//
///*
//3 OP CODE:
//
//
//
//
//*/
//
//
//
///*
//
//
//	- For parameters: create map of parameter to offset from sp
//
//		-> i.e. f(int x, int y) => x:0, y:4
//		-> do this for arrays and structs
//
//	- when parameter is referenced, load into register based on offset
//	
//
//	- before calling function, push all arguments to the stack?
//	- after calling function, pop all arguments?
//
//
//
//	When returning a struct, have caller allocate space on stack for the struct, then copy data into that space on the stack
//	and return a pointer to the buffer.
//
//*/
//
///* 
//	Have toString() function for ArmStatement struct
//
//	format:
//
//	string command = GetCommandFromStatementType(type) + " "; //also add flags and conditional execution to this
//	for (const auto& arg : arguments)
//	{
//		command = command + arg.toString() + ", ";
//	}
//
//	command = command.substr(0, command.size() - 2); //cut off the last commma and space"
//
//*/