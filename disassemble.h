#ifndef DISASSEMBLE_H
#define DISASSEMBLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <BaseTsd.h>

#define		MAX_MNEMONIC_LEN		32
#define		MAX_OPERAND_LEN		64
#define		MAX_INSTRUCTION_LEN	128

#define  ALL_PRINT       1   // 打印所有指令
#define  BLOCK_PRINT     2   //打印基本块的开始指令
#define  CALL_PRINT      3   //只打印call指令

typedef struct _INSTRUCTION
{
	/* prefixes */

	char	RepeatPrefix;
	char	SegmentPrefix;
	char	OperandPrefix;
	char	AddressPrefix;

	/* opcode */

	unsigned int	Opcode;

	/* ModR/M */

	char	ModRM;

	/* SIB */

	char	SIB;

	/* Displacement */

	unsigned int	Displacement;

	/* Immediate */

	unsigned int	Immediate;
	
	/* Linear address of this instruction */

	unsigned int	LinearAddress;
	
	/* Flags */

	char dFlag, wFlag, sFlag;


} INSTRUCTION, *PINSTRUCTION;


unsigned char *ParseModRM(unsigned char *Code, PINSTRUCTION Instruction, char *OperandRM);
unsigned char *ParseImmediate(unsigned char *Code, PINSTRUCTION Instruction, char *OperandImmediate);
unsigned char *ParseSIB(unsigned char *Code, PINSTRUCTION Instruction, char *SIBStr);
unsigned char *ParseRegModRM(unsigned char *Code, PINSTRUCTION Instruction, char *Operand1, char *Operand2);
unsigned char *Disassemble(unsigned int LinearAddress, unsigned char *Code, PINSTRUCTION Instruction, char *InstructionStr);

//addbyxqx 20110702
int GetOp1Addr(int *outaddr);

#endif