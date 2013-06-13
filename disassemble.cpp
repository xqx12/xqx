
#include "disassemble.h"
//#include "taintlist.h"



const char *RepeatPrefixes[] = {"lock", "rep", "repe", "repz", "repne", "repnz"};
const char *AddressPrefixes[] = {"qword", "fword", "dword", "word", "byte"};
const char *SegmentRegisters[] = {"es", "cs", "ss", "ds", "fs", "gs"};
const char *Register32[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
const char *Register16[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
const char *Register8[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
const char *Address16[] = {"bx+si", "bx+di", "bp+si", "bp+di", "si", "di", "bp", "bx"};

const char *ArithmeticMnemonic[] = {"add", "or", "adc", "sbb", "and", "sub", "xor", "cmp" };
const char *BCDAdjustMnemonic[] = {"daa", "das", "aaa", "aas", "aam", "aad"};
const char *JxxxMnemonic[] = {"jo", "jb", "jz", "jbe", "js", "jp", "jl", "jle"};
const char *JnxxMnemonic[] = {"jno", "jnb", "jnz", "ja", "jns", "jnp", "jge", "jg"};
const char *StrMnemonic[] = {"", "", "movs", "cmps", "", "stos", "lods", "scas"};
const char *LogicalShiftMnemonic[] = {"rol", "ror", "rcl", "rcr", "shl", "shr", "sal", "sar"};
const char *LoopMnemonic[] = {"loopnz", "loopz", "loop"};
const char *LogicalArithmeticMnemonic[] = {"test", "???", "not", "neg", "mul", "imul", "div", "idiv"};
const char *FlagMnemonic[] = {"clc", "stc", "cli", "sti", "cld", "std"};
const char *FFOpcodeMnemonic[] = {"inc", "dec", "call", "call", "jmp", "jmp", "push"};



//---------------addbyxqx20110702-----------------
unsigned int bNeedPrint;
unsigned int Op1Addr;
unsigned int Op2Addr;

extern unsigned int  dwPrintLevel;
extern unsigned int  dwTraceLevel;

//获取第一操作数的地址
int GetOp1Addr(int *outaddr)
{
	*outaddr = Op1Addr;
	return 1;

}

//---------------addbyxqx20110702-----------------

unsigned char *Disassemble(unsigned int LinearAddress, unsigned char *Code, PINSTRUCTION Instruction, char *InstructionStr)
{
	char prefix[MAX_OPERAND_LEN] = "";
	char mnemonic[MAX_MNEMONIC_LEN] = "";
	char operand1[MAX_OPERAND_LEN] = "";
	char operand2[MAX_OPERAND_LEN] = "";
	char operand3[MAX_OPERAND_LEN] = "";
	
	unsigned char *currentCode;
	char prefixFlag; 

	//--add by xqx 110628--------------------
	unsigned int nTmp;
	unsigned char cTmp;

	bNeedPrint = 0;
	Op1Addr = 0;

	//--add by xqx 110628--------------------

	currentCode = Code;
	prefixFlag = 0;

	
	/* initialize Instrction structure */

	Instruction->RepeatPrefix = -1;
	Instruction->SegmentPrefix = -1;
	Instruction->OperandPrefix = -1;
	Instruction->AddressPrefix = -1;

	Instruction->Opcode = -1;
	Instruction->ModRM = -1;
	Instruction->SIB = -1;
	
	Instruction->Displacement = -1;
	Instruction->Immediate = -1;

	Instruction->dFlag = Instruction->wFlag = Instruction->sFlag = -1;

	Instruction->LinearAddress = LinearAddress;

	/* check PREFIXES and save the values, note that prefixes in the same group can only appear once */
	/* and the values assigned to xxxxPrefix in Instruction structure are the indexes of the names */
	/* in corresponding name tables */
	
	while(*currentCode == 0xF0 || *currentCode == 0xF2 || *currentCode == 0xF3 ||
		*currentCode == 0x2E || *currentCode == 0x36 || *currentCode == 0x3E ||
		*currentCode == 0x26 || *currentCode == 0x64 || *currentCode == 0x65 ||
		*currentCode == 0x66 || *currentCode == 0x67)
	{
		switch(*currentCode)
		{
			/* group1: lock and repeat prefixes */

			case 0xF0 :
			{	
				if(prefixFlag)
				{
					strcpy(InstructionStr, "prefix lock:");
					currentCode++;
					return currentCode;
				}
				else
				{
					/* rescan */

					if(Instruction->RepeatPrefix >= 0)
					{
						currentCode = Code;
						prefixFlag = 1;
					}
					else
					{
						Instruction->RepeatPrefix = 0;
						currentCode++;
					}
				}
				
				break;
			}
			case 0xF2 :
			{
				if(prefixFlag)
				{
					strcpy(InstructionStr, "prefix repnz:");
					currentCode++;
					return currentCode;
				}
				else
				{
					/* rescan */

					if(Instruction->RepeatPrefix >= 0)
					{
						currentCode = Code;
						prefixFlag = 1;
					}
					else
					{
						Instruction->RepeatPrefix = 5;
						currentCode++;
					}
				}

				break;
			}
			case 0xF3 :
			{
				if(prefixFlag)
				{
					strcpy(InstructionStr, "prefix rep:");
					currentCode++;
					return currentCode;
				}
				else
				{
					/* rescan */

					if(Instruction->RepeatPrefix >= 0)
					{
						currentCode = Code;
						prefixFlag = 1;
					}
					else
					{
						Instruction->RepeatPrefix = 1;
						currentCode++;
					}
				}

				break;
			}

			/* group2: segment override prefixes */

			case 0x2E :
			{
				
				if(prefixFlag)
				{
					strcpy(InstructionStr, "prefix cs:");
					currentCode++;
					return currentCode;
				}
				else
				{
					/* rescan */

					if(Instruction->SegmentPrefix >= 0)
					{
						currentCode = Code;
						prefixFlag = 1;
					}
					else
					{
						Instruction->SegmentPrefix = 1;
						currentCode++;
					}
				}

				break;
			}
			case 0x36 :
			{
				if(prefixFlag)
				{
					strcpy(InstructionStr, "prefix ss:");
					currentCode++;
					return currentCode;
				}
				else
				{
					/* rescan */

					if(Instruction->SegmentPrefix >= 0)
					{
						currentCode = Code;
						prefixFlag = 1;
					}
					else
					{
						Instruction->SegmentPrefix = 2;
						currentCode++;
					}
				}

				break;
			}
			case 0x3E :
			{
				if(prefixFlag)
				{
					strcpy(InstructionStr, "prefix ds:");
					currentCode++;
					return currentCode;
				}
				else
				{
					/* rescan */

					if(Instruction->SegmentPrefix >= 0)
					{
						currentCode = Code;
						prefixFlag = 1;
					}
					else
					{
						Instruction->SegmentPrefix = 3;
						currentCode++;
					}
				}

				break;
			}
			case 0x26 :
			{	
				if(prefixFlag)
				{
					strcpy(InstructionStr, "prefix es:");
					currentCode++;
					return currentCode;
				}
				else
				{
					/* rescan */

					if(Instruction->SegmentPrefix >= 0)
					{
						currentCode = Code;
						prefixFlag = 1;
					}
					else
					{
						Instruction->SegmentPrefix = 0;
						currentCode++;
					}
				}

				break;
			}
			case 0x64 :
			{
				if(prefixFlag)
				{
					strcpy(InstructionStr, "prefix fs:");
					currentCode++;
					return currentCode;
				}
				else
				{
					/* rescan */
				
					if(Instruction->SegmentPrefix >= 0)
					{
						currentCode = Code;
						prefixFlag = 1;
					}
					else
					{
						Instruction->SegmentPrefix = 4;
						currentCode++;
					}
				}

				break;
			}
			case 0x65 :
			{	
				if(prefixFlag)
				{
					strcpy(InstructionStr, "prefix gs:");
					currentCode++;
					return currentCode;
				}
				else
				{
					/* rescan */

					if(Instruction->SegmentPrefix >= 0)
					{
						currentCode = Code;
						prefixFlag = 1;
					}
					else
					{
						Instruction->SegmentPrefix = 5;
						currentCode++;
					}
				}

				break;
			}

			/* group3: Operand-size override prefixes */

			case 0x66 :
			{
				if(prefixFlag)
				{
					strcpy(InstructionStr, "prefix datasize:");
					currentCode++;
					return currentCode;
				}
				else
				{
					/* rescan */

					if(Instruction->OperandPrefix >= 0)
					{
						currentCode = Code;
						prefixFlag = 1;
					}
					else
					{
						Instruction->OperandPrefix = 0;
						currentCode++;
					}
				}

				break;
			}

			/* group4: Address-size override prefixes */

			case 0x67 :
			{
				if(prefixFlag)
				{
					strcpy(InstructionStr, "prefix addrsize:");
					currentCode++;
					return currentCode;
				}
				else
				{
					/* rescan */

					if(Instruction->AddressPrefix >= 0)
					{
						currentCode = Code;
						prefixFlag = 1;
					}
					else
					{
						Instruction->AddressPrefix = 0;
						currentCode++;
					}
				}
				
				break;
			}
		}
	}

	/* CODE map */

	if (dwPrintLevel == ALL_PRINT)
	{
		bNeedPrint = 1;
	}
	switch(*currentCode)
	{
		/* Arithmetic operations */

		case 0x00: case 0x01: case 0x02: case 0x03: 
		case 0x08: case 0x09: case 0x0A: case 0x0B: 
		case 0x10: case 0x11: case 0x12: case 0x13: 
		case 0x18: case 0x19: case 0x1A: case 0x1B: 
		case 0x20: case 0x21: case 0x22: case 0x23: 
		case 0x28: case 0x29: case 0x2A: case 0x2B: 
		case 0x30: case 0x31: case 0x32: case 0x33: 
		case 0x38: case 0x39: case 0x3A: case 0x3B: 
		{
			Instruction->Opcode = *currentCode;
			Instruction->dFlag = (*currentCode >> 1) & 1;
			Instruction->wFlag = (*currentCode) & 1;

			sprintf(mnemonic, ArithmeticMnemonic[(*currentCode >> 3) & 0x1F]);
			
			currentCode++;
			currentCode = ParseRegModRM(currentCode, Instruction, operand1, operand2);
			
			break;
		}
		case 0x04: case 0x05: case 0x0C: case 0x0D:	
		case 0x14: case 0x15: case 0x1C: case 0x1D:
		case 0x24: case 0x25: case 0x2C: case 0x2D:
		case 0x34: case 0x35: case 0x3C: case 0x3D:
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = (*currentCode & 1);
			Instruction->sFlag = ! Instruction->wFlag;

			sprintf(mnemonic, ArithmeticMnemonic[(*currentCode >> 3) & 0x1F]);
			
			sprintf(operand1, "%s", Instruction->wFlag ? (Instruction->OperandPrefix >= 0 ? "ax" : "eax") 
				 : "al");

			currentCode++;
			currentCode = ParseImmediate(currentCode, Instruction, operand2);
			
			break;
		}
		case 0x80: case 0x81: case 0x82: case 0x83:
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = (*currentCode & 1);
			Instruction->sFlag = (*currentCode >> 1) & 1;
			
			/* special cases */

			if(*currentCode == 0x80)
			{
				Instruction->sFlag = 1;
			}

			sprintf(mnemonic, ArithmeticMnemonic[(*(currentCode + 1) >> 3) & 7]);

			currentCode++;
			currentCode = ParseModRM(currentCode, Instruction, operand1);
			currentCode = ParseImmediate(currentCode, Instruction, operand2);

			/* special cases */

			if(Instruction->Opcode == 0x83)
			{
				sprintf(operand2, "%X%X", (Instruction->Immediate >> 7 & 1) ? 0xFFFF : 0x0, Instruction->Immediate);
			}

			break;
		}
		
		/* push pop operations */

		case 0x06: case 0x07 : case 0x0E: case 0x16: case 0x17: case 0x1E : case 0x1F:
		{
			Instruction->Opcode = *currentCode;

			sprintf(mnemonic, "%s", *currentCode & 1 ? "pop" : "push");
			sprintf(operand1, "%s", SegmentRegisters[(*currentCode >> 3) & 3]);
			currentCode++;

			break;
		}
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F:
		{
			Instruction->Opcode = *currentCode;

			sprintf(mnemonic, "%s", (*currentCode >> 3) & 1 ? "pop" : "push");
			sprintf(operand1, "%s", Instruction->OperandPrefix >= 0 ? Register16[*currentCode & 7] : Register32[*currentCode & 7]);
			currentCode++;
			
			break;
		}
		case 0x60: case 0x61: case 0x9C: case 0x9D:
		{
			Instruction->Opcode = *currentCode;

			sprintf(mnemonic, "%s%c%c", (*currentCode & 1) ? "pop" : "push", 
				(*currentCode >> 7) & 1 ? 'f' : 'a', Instruction->OperandPrefix >= 0 ? 'w' : 'd');
			currentCode++;

			break;
		}
		case 0x68: case 0x6A:
		{
			Instruction->Opcode = *currentCode;
			Instruction->sFlag = (*currentCode >> 1) & 1;
			
			sprintf(mnemonic, "%s", "push");
			currentCode++;
			currentCode = ParseImmediate(currentCode, Instruction, operand1);

			break;
		}
		case 0x8F:
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = (*currentCode) & 1;

			sprintf(mnemonic, "%s", "pop");
			currentCode++;
			currentCode = ParseModRM(currentCode, Instruction, operand1);

			break;
		}
		case 0x27: case 0x2F: case 0x37: case 0x3F:
		{
			Instruction->Opcode = *currentCode;

			sprintf(mnemonic, "%s", BCDAdjustMnemonic[(*currentCode >> 3) & 3]);
			currentCode++;

			break;
		}

		/* inc and dec */
		
		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F:
		{
			Instruction->Opcode = *currentCode;

			sprintf(mnemonic, "%s", (*currentCode >> 3) & 1 ? "dec" : "inc");
			sprintf(operand1, "%s", Instruction->OperandPrefix >= 0 ? Register16[*currentCode & 7] : Register32[*currentCode & 7]);
			currentCode++;
			
			break;
		}

		/* bound */

		case 0x62:
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = *currentCode & 1;
			Instruction->sFlag = (*currentCode >> 1) & 1;

			sprintf(mnemonic, "%s", "bound");
			sprintf(operand1, "%s", Instruction->OperandPrefix >= 0 ? Register16[(*(currentCode + 1) >> 3) & 7] :
				Register32[(*(currentCode + 1) >> 3) & 7]);
			currentCode++;
			currentCode = ParseModRM(currentCode, Instruction, operand2);
			
			/* special case, the size of memory address must be twice the size of register */

			sprintf(operand3, "%s%s", Instruction->OperandPrefix >= 0 ? "dword ptr" : "qword ptr",
				strstr(operand2, "ptr") + 3);
			sprintf(operand2, "%s", operand3);
			*operand3 = '\0';

			break;
		}

		/* arpl */

		case 0x63:
		{
			Instruction->Opcode = *currentCode;

			/* special case, the operand size of arpl must be 16-bits */

			Instruction->OperandPrefix = 0;
			Instruction->wFlag = 1;
			Instruction->dFlag = 0;

			sprintf(mnemonic, "%s", "arpl");
			currentCode++;
			currentCode = ParseRegModRM(currentCode, Instruction, operand1, operand2);

			break;
		}
		
		/* imul */

		case 0x69: case 0x6B:
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = *currentCode & 1;
			Instruction->sFlag = (*currentCode >> 1) & 1;
			Instruction->dFlag = (*currentCode >> 1) & 1;

			sprintf(mnemonic, "%s", "imul");
			currentCode++;
			currentCode = ParseRegModRM(currentCode, Instruction, operand1, operand2);
			currentCode = ParseImmediate(currentCode, Instruction, operand3);

			break;
		}
		
		/* ins and outs */

		case 0x6C: case 0x6D: case 0x6E: case 0x6F:
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = *currentCode & 1;
			Instruction->dFlag = (*currentCode >> 1) & 1;
			
			sprintf(prefix, "%s", Instruction->RepeatPrefix > 0 ? RepeatPrefixes[Instruction->RepeatPrefix] : "");
			sprintf(mnemonic, "%s%c", Instruction->dFlag ? "outs" : "ins", Instruction->wFlag ? 
				(Instruction->OperandPrefix >= 0 ? 'w' : 'd') : 'b');
			currentCode++;

			break;
		}
		
		/* jmp instructions j(n)xx */
		
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F:
		{
			Instruction->Opcode = *currentCode;
			
			sprintf(mnemonic, "%s", *currentCode & 1 ? JnxxMnemonic[(*currentCode >> 1) & 7] : JxxxMnemonic[(*currentCode >> 1) & 7]);
			currentCode++;
			sprintf(operand1, "short %X", Instruction->LinearAddress + *((char*)currentCode) + currentCode - Code + 1);
			currentCode++;

			// 增加是否跟踪打印的判断 [3/31/2012 xqx]
			if (dwPrintLevel == BLOCK_PRINT)
			{
				bNeedPrint = 1;
			}

			break;
		}

		/* test */

		case 0x84: case 0x85:
		{
			Instruction->Opcode = *currentCode;
			Instruction->dFlag = (*currentCode >>1) & 1;
			Instruction->wFlag = (*currentCode) & 1;

			sprintf(mnemonic, "%s", "test");
			currentCode++;
			currentCode = ParseRegModRM(currentCode, Instruction, operand1, operand2);

			break;
		}

		/* xchg */

		case 0x86: case 0x87:
		{
			Instruction->Opcode = *currentCode;
			Instruction->dFlag = (*currentCode >>1) & 1;
			Instruction->wFlag = (*currentCode) & 1;

			sprintf(mnemonic, "%s", "xchg");
			currentCode++;
			currentCode = ParseRegModRM(currentCode, Instruction, operand1, operand2);
			
			break;
		}
		case 0x90:
		{
			Instruction->Opcode = *currentCode;
			
			sprintf(mnemonic, "%s", Instruction->RepeatPrefix == 1 ? "pause" : "nop");
			currentCode++;

			break;
		}
		case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		{
			Instruction->Opcode = *currentCode;

			sprintf(mnemonic, "%s", "xchg");
			sprintf(operand1, "%s", Instruction->OperandPrefix >= 0 ? Register16[*currentCode & 7] : Register32[*currentCode & 7]);
			sprintf(operand2, "%s", Instruction->OperandPrefix >= 0 ? "ax" :"eax");
			currentCode++;

			break;
		}

		/* mov */

		case 0x88: case 0x89: case 0x8A: case 0x8B:
		{
			Instruction->Opcode = *currentCode;
			Instruction->dFlag = (*currentCode >>1) & 1;
			Instruction->wFlag = (*currentCode) & 1;

			sprintf(mnemonic, "%s", "mov");
			currentCode++;
			currentCode = ParseRegModRM(currentCode, Instruction, operand1, operand2);
			
			//bNeedPrint = 1;
			break;
		}
		case 0x8C: case 0x8E:
		{
			Instruction->Opcode = *currentCode;
			Instruction->dFlag = (*currentCode >> 1) & 1;

			/* special cases code segment registers are 16-bits long */
			
			Instruction->OperandPrefix = 0;
			Instruction->wFlag = 1;

			sprintf(mnemonic, "%s", "mov");
			currentCode++;
			sprintf(Instruction->dFlag ? operand1 : operand2, "%s", SegmentRegisters[(*currentCode >> 3) & 7]);
			currentCode = ParseModRM(currentCode, Instruction, Instruction->dFlag ? operand2 : operand1);

			//bNeedPrint = 1;
			break;
		}
		
		case 0xA0: case 0xA1: case 0xA2: case 0xA3:
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = *currentCode & 1;
			Instruction->dFlag = (*currentCode >> 1) & 1;

			sprintf(mnemonic, "%s", "mov");
			currentCode++;
			sprintf(Instruction->dFlag ? operand1 : operand2, "%s%s%s[%X]", Instruction->wFlag ? (Instruction->OperandPrefix >= 0 ? "word ptr " : "dword ptr ") : "byte ptr ",
				Instruction->SegmentPrefix >= 0 ? SegmentRegisters[Instruction->SegmentPrefix] : "", 
				Instruction->SegmentPrefix >= 0 ? ":" : "",
				*((unsigned int *)currentCode));
			sprintf(Instruction->dFlag ? operand2 : operand1, "%s", Instruction->wFlag ? (Instruction->OperandPrefix >= 0 ? "ax" : "eax") : "al");
			currentCode += 4;

			break;
		}
		case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7:
		case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBE: case 0xBF:
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = (*currentCode >>3) & 1;
			Instruction->sFlag = !(Instruction->wFlag);

			sprintf(mnemonic, "%s", "mov");
			sprintf(operand1, Instruction->wFlag ? Instruction->OperandPrefix >= 0 ? Register16[*currentCode & 7] : Register32[*currentCode & 7] : Register8[*currentCode & 7]);
			currentCode++;
			currentCode = ParseImmediate(currentCode, Instruction, operand2);
			
			//bNeedPrint = 1;
			break;
		}

		/* lea */

		case 0x8D:
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = (*currentCode) & 1;

			/*special cases */

			Instruction->dFlag = 1;
			
			sprintf(mnemonic, "%s", "lea");
			currentCode++;	
			currentCode = ParseRegModRM(currentCode, Instruction, operand1, operand2);

			break;
		}
		
		/* cbw, cwd, cwde, cdq*/

		case 0x98:
		{
			Instruction->Opcode = *currentCode;
			
			sprintf(mnemonic, "%s", Instruction->OperandPrefix >= 0 ? "cbw" : "cwde");
			currentCode++;

			break;
		}
		case 0x99:
		{
			Instruction->Opcode = *currentCode;

			sprintf(mnemonic, "%s", Instruction->OperandPrefix >= 0 ? "cwd" : "cdq");
			currentCode++;

			break;
		}

		/* call far ptr16 : 32 */

		case 0x9A:
		{
			Instruction->Opcode = *currentCode;
			Instruction->sFlag = 0;

			sprintf(mnemonic, "%s", "call");
			currentCode++;
			currentCode = ParseImmediate(currentCode, Instruction, operand1);
			Instruction->OperandPrefix = 0;
			currentCode = ParseImmediate(currentCode, Instruction, operand2);
			sprintf(operand3, "far %s : %s", operand2, operand1);
			strncpy(operand1, operand3, MAX_OPERAND_LEN);
			*operand2 = '\0';
			*operand3 = '\0';

			// 增加是否跟踪打印的判断 [3/31/2012 xqx]
			if (dwPrintLevel == BLOCK_PRINT || dwPrintLevel == CALL_PRINT)
			{
				bNeedPrint = 1;
			}

			break;
		}

		/* wait */

		case 0x9B:
		{
			Instruction->Opcode = *currentCode;

			sprintf(mnemonic, "%s", "wait");
			currentCode++;

			break;
		}
		
		/* lahf and sahf */

		case 0x9E:
		{
			Instruction->Opcode = *currentCode;
			
			sprintf(mnemonic, "%s", "ashf");
			currentCode++;

			break;
		}
		case 0x9F:
		{
			Instruction->Opcode = *currentCode;

			sprintf(mnemonic, "%s", "lahf");
			currentCode++;
			
			break;
		}
		
		/* string operations */

		case 0xA4: case 0xA5: case 0xA6: case 0xA7:
		case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAE: case 0xAF:
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = *currentCode & 1;
			
			sprintf(prefix, "%s", Instruction->RepeatPrefix > 0 ? RepeatPrefixes[Instruction->RepeatPrefix] : "");
			sprintf(mnemonic, "%s%c", StrMnemonic[(*currentCode >> 1) & 7], Instruction->wFlag ? (Instruction->OperandPrefix >= 0 ? 'w' : 'd') : 'b');
			currentCode++;

			break;
		}

		/* test */

		case 0xA8: case 0xA9:
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = *currentCode & 1;
			Instruction->sFlag = !(*currentCode & 1);

			sprintf(mnemonic, "%s", "test");
			sprintf(operand1, "%s", Instruction->wFlag ? (Instruction->OperandPrefix >= 0 ? "ax" : "eax") : "al");
			currentCode++;
			currentCode = ParseImmediate(currentCode, Instruction, operand2);
			
			break;
		}
		
		/* logical shift */

		case 0xC0: case 0xC1:
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = *currentCode & 1;
			Instruction->sFlag = 1;
			
			currentCode++;
			sprintf(mnemonic, "%s", LogicalShiftMnemonic[(*currentCode >> 3) & 7]);
			currentCode = ParseModRM(currentCode, Instruction, operand1);
			currentCode = ParseImmediate(currentCode, Instruction, operand2);

			break;
		}
		
		/* retn */

		case 0xC2 :
		{
			Instruction->Opcode = *currentCode;
			Instruction->sFlag = 0;
			Instruction->OperandPrefix = 0;

			sprintf(mnemonic, "retn");
			currentCode++;
			currentCode = ParseImmediate(currentCode, Instruction, operand1);

			// 增加是否跟踪打印的判断 [3/31/2012 xqx]
			if (dwPrintLevel == BLOCK_PRINT || dwPrintLevel == CALL_PRINT)
			{
				bNeedPrint = 1;
			}

			break;
		}
		case 0xC3:
		{
			Instruction->Opcode = *currentCode;
			
			sprintf(mnemonic, "retn");
			currentCode++;

			// 增加是否跟踪打印的判断 [3/31/2012 xqx]
			if (dwPrintLevel == BLOCK_PRINT || dwPrintLevel == CALL_PRINT)
			{
				bNeedPrint = 1;
			}

			break;
		}
		
		/* les, lds */

		case 0xC4: 
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = 1;
			Instruction->dFlag = 1;

			sprintf(mnemonic, "les");
			currentCode++;
			currentCode = ParseRegModRM(currentCode, Instruction, operand1, operand2);
			sprintf(operand3, "%s%s", Instruction->OperandPrefix >= 0 ? "dword ptr" : "fword ptr", strstr(operand2, "ptr") + 3);
			sprintf(operand2, "%s", operand3);
			*operand3 = '\0';

			break;
		}
		case 0xC5:
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = 1;
			Instruction->dFlag = 1;

			sprintf(mnemonic, "lds");
			currentCode++;
			currentCode = ParseRegModRM(currentCode, Instruction, operand1, operand2);
			sprintf(operand3, "%s%s", Instruction->OperandPrefix >= 0 ? "dword ptr" : "fword ptr", strstr(operand2, "ptr") + 3);
			sprintf(operand2, "%s", operand3);
			*operand3 = '\0';

			break;
		}
		case 0xC6: case 0xC7:
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = *currentCode & 1;
			Instruction->sFlag = ((*currentCode & 1) ^ 1) & 1;

			sprintf(mnemonic, "mov");
			currentCode++;
			currentCode = ParseModRM(currentCode, Instruction, operand1);
			currentCode = ParseImmediate(currentCode, Instruction, operand2);

			break;
		}
		
		/* enter leave */

		case 0xC8:
		{
			Instruction->Opcode = *currentCode;
			Instruction->sFlag = 0;
			Instruction->OperandPrefix = 0;

			sprintf(mnemonic, "enter");
			currentCode++;
			currentCode = ParseImmediate(currentCode, Instruction, operand1);
			Instruction->sFlag = 1;
			currentCode = ParseImmediate(currentCode, Instruction, operand2);

			break;
		}
		case 0xC9:
		{
			Instruction->Opcode = *currentCode;
			
			sprintf(mnemonic, "leave");
			currentCode++;

			break;
		}

		/* retx */

		case 0xCA:
		{
			Instruction->Opcode = *currentCode;
			Instruction->sFlag = 0;
			Instruction->OperandPrefix = 0;

			sprintf(mnemonic, "retf");
			currentCode++;
			currentCode = ParseImmediate(currentCode, Instruction, operand1);

			// 增加是否跟踪打印的判断 [3/31/2012 xqx]
			if (dwPrintLevel == BLOCK_PRINT || dwPrintLevel == CALL_PRINT)
			{
				bNeedPrint = 1;
			}
			
			break;
		}
		case 0xCB:
		{
			Instruction->Opcode = *currentCode;

			sprintf(mnemonic, "retf");
			currentCode++;

			// 增加是否跟踪打印的判断 [3/31/2012 xqx]
			if (dwPrintLevel == BLOCK_PRINT || dwPrintLevel == CALL_PRINT)
			{
				bNeedPrint = 1;
			}

			break;
		}
		
		/* int */

		case 0xCC:
		{
			Instruction->Opcode = *currentCode;

			sprintf(mnemonic, "int3");
			currentCode++;

			break;
		}
		case 0xCD:
		{
			Instruction->Opcode = *currentCode;
			Instruction->sFlag = 1;

			sprintf(mnemonic, "int");
			currentCode++;
			currentCode = ParseImmediate(currentCode, Instruction, operand1);
			
			break;
		}
		case 0xCE:
		{
			Instruction->Opcode = *currentCode;
			
			sprintf(mnemonic, "into");
			currentCode++;

			break;
		}
		case 0xCF:
		{
			Instruction->Opcode = *currentCode;

			sprintf(mnemonic, "%s%c", "iret", Instruction->OperandPrefix >= 0 ? 'w' : 'd');
			currentCode++;

			break;
		}

		/* rol ror rcl rcr shl sal shr shl sar */
		
		case 0xD0: case 0xD1:
		{
			Instruction->Opcode  = *currentCode;
			Instruction->wFlag = *currentCode & 1;

			currentCode++;
			sprintf(mnemonic, "%s", LogicalShiftMnemonic[(*currentCode >> 3) & 7]);
			currentCode = ParseModRM(currentCode, Instruction, operand1);
			sprintf(operand2, "1");

			break;
		}
		case 0xD2: case 0xD3:
		{
			Instruction->Opcode  = *currentCode;
			Instruction->wFlag = *currentCode & 1;

			currentCode++;
			sprintf(mnemonic, "%s", LogicalShiftMnemonic[(*currentCode >> 3) & 7]);
			currentCode = ParseModRM(currentCode, Instruction, operand1);
			sprintf(operand2, "cl");

			break;
		}

		/* aam aad */

		case 0xD4: case 0xD5:
		{
			Instruction->Opcode = *currentCode;
			Instruction->sFlag = 1;

			sprintf(mnemonic, BCDAdjustMnemonic[*currentCode & 7]);
			currentCode++;
			if(*currentCode == 0x0A)
			{
				currentCode++;
			}
			else
			{
				currentCode = ParseImmediate(currentCode, Instruction, operand1);
			}

			break;
		}
		
		/* setalc */

		case 0xD6:
		{
			Instruction->Opcode = *currentCode;
			
			sprintf(mnemonic, "salc");
			currentCode++;

			break;
		}
		case 0xD7:
		{
			Instruction->Opcode = *currentCode;

			sprintf(mnemonic, "xlat");
			sprintf(operand1, "byte ptr %s%s[ebx + al]", Instruction->SegmentPrefix >= 0 ? SegmentRegisters[Instruction->SegmentPrefix] : "", Instruction->SegmentPrefix >= 0 ? ":" : "");
			currentCode++;

			break;
		}
		
		/* loopxx */
		
		case 0xE0: case 0xE1: case 0xE2:
		{
			Instruction->Opcode = *currentCode;

			sprintf(mnemonic, "%s", LoopMnemonic[*currentCode & 7]);
			currentCode++;
			sprintf(operand1, "short %X", Instruction->LinearAddress + *((char*)currentCode) + currentCode - Code + 1);
			currentCode++;

			break;
		}
		case 0xE3:
		{
			Instruction->Opcode = *currentCode;

			sprintf(mnemonic, "%s", "jcxz");
			currentCode++;
			sprintf(operand1, "short %X", Instruction->LinearAddress + *((char*)currentCode) + currentCode - Code + 1);
			currentCode++;

			break;
		}
		
		/* in out */

		case 0xE4: case 0xE5: case 0xE6: case 0xE7:
		{
			Instruction->Opcode = *currentCode;
			Instruction->dFlag = (*currentCode >> 1) & 1;
			Instruction->wFlag = *currentCode & 1;
			Instruction->sFlag = 1;
			
			currentCode++;
			sprintf(mnemonic, "%s", Instruction->dFlag ? "out" : "in");
			currentCode = ParseImmediate(currentCode, Instruction, Instruction->dFlag ? operand1 : operand2);
			sprintf(Instruction->dFlag ? operand2 : operand1, "%s", Instruction->wFlag ? (Instruction->OperandPrefix >= 0 ? "ax" : "eax") : "al");

			break;
		}
		case 0xEC: case 0xED: case 0xEE: case 0xEF:
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = *currentCode & 1;
			Instruction->dFlag = (*currentCode >> 1) & 1;

			sprintf(mnemonic, "%s", Instruction->dFlag ? "out" : "in");
			sprintf(Instruction->dFlag ? operand2 : operand1, "%s", Instruction->wFlag ? (Instruction->OperandPrefix >= 0 ? "ax" : "eax") : "al");
			sprintf(Instruction->dFlag ? operand1 : operand2, "dx");

			currentCode++;

			break;
		}

		/* call jmp*/

		case 0xE8:
		{
			Instruction->Opcode = *currentCode;
			Instruction->sFlag;

			sprintf(mnemonic, "call");
			currentCode++;

			if(Instruction->OperandPrefix >= 0)
			{
				sprintf(operand1, "%X", (Instruction->LinearAddress & 0xFFFF) + (*((int *)currentCode) & 0xFFFF) + currentCode - Code + 2);
				currentCode += 2;
			}
			else
			{
				sprintf(operand1, "%X", Instruction->LinearAddress + *((int *)currentCode) + currentCode - Code + 4);
				currentCode += 4;
			}

			// 增加是否跟踪打印的判断 [3/31/2012 xqx]
			if (dwPrintLevel == BLOCK_PRINT || dwPrintLevel == CALL_PRINT)
			{
				bNeedPrint = 1;
			}

			break;
		}
		case 0xE9:
		{
			Instruction->Opcode = *currentCode;
			Instruction->sFlag;

			sprintf(mnemonic, "jmp");
			currentCode++;

			if(Instruction->OperandPrefix >= 0)
			{
				sprintf(operand1, "%X", (Instruction->LinearAddress & 0xFFFF) + (*((int *)currentCode) & 0xFFFF) + currentCode - Code + 2);
				currentCode += 2;
			}
			else
			{
				sprintf(operand1, "%X", Instruction->LinearAddress + *((int *)currentCode) + currentCode - Code + 4);
				currentCode += 4;
			}

			// 增加是否跟踪打印的判断 [3/31/2012 xqx]
			if (dwPrintLevel == BLOCK_PRINT )
			{
				bNeedPrint = 1;
			}

			break;
		}
		case 0xEA:
		{
			Instruction->Opcode = *currentCode;
			Instruction->sFlag = 0;

			sprintf(mnemonic, "jmp");
			currentCode++;
			currentCode = ParseImmediate(currentCode, Instruction, operand2);
			Instruction->OperandPrefix = 0;
			currentCode = ParseImmediate(currentCode, Instruction, operand3);
			sprintf(operand1, "far %s:%s", operand3, operand2);
			*operand2 = '\0';
			*operand3 = '\0';

			// 增加是否跟踪打印的判断 [3/31/2012 xqx]
			if (dwPrintLevel == BLOCK_PRINT )
			{
				bNeedPrint = 1;
			}

			break;
		}
		case 0xEB:
		{
			Instruction->Opcode = *currentCode;
			Instruction->sFlag = 1;

			sprintf(mnemonic, "jmp");
			currentCode++;
			sprintf(operand1, "short %X", Instruction->LinearAddress + *currentCode + currentCode - Code + 1);
			currentCode++;

			// 增加是否跟踪打印的判断 [3/31/2012 xqx]
			if (dwPrintLevel == BLOCK_PRINT )
			{
				bNeedPrint = 1;
			}

			break;
		}
		
		case 0xF1:
		{
			Instruction->Opcode = *currentCode;
			
			sprintf(mnemonic, "int1");
			currentCode++;

			break;
		}
		case 0xF4:
		{
			Instruction->Opcode = *currentCode;

			sprintf(mnemonic, "hlt");
			currentCode++;

			break;
		}
		case 0xF5:
		{
			Instruction->Opcode = *currentCode;

			sprintf(mnemonic, "cmc");
			currentCode++;

			break;
		}
		case 0xF6: case 0xF7:
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = *currentCode & 1;
			Instruction->sFlag = (~(*currentCode & 1)) & 1;
			
			currentCode++;
			sprintf(mnemonic, "%s", LogicalArithmeticMnemonic[(*currentCode >> 3) & 7]);
			currentCode = ParseModRM(currentCode, Instruction, operand1);
			if(strcmp(mnemonic, "test") == 0)
			{
				currentCode = ParseImmediate(currentCode, Instruction, operand2);
			}

			break;
		}
		
		/* flag set */

		case 0xF8: case 0xF9: case 0xFA: case 0xFB: case 0xFC: case 0xFD:
		{
			Instruction->Opcode = *currentCode;

			sprintf(mnemonic, "%s", FlagMnemonic[*currentCode & 7]);
			currentCode++;

			break;
		}
		
		case 0xFE:
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = *currentCode & 1;

			currentCode++;
			sprintf(mnemonic, "%s", (*currentCode >> 3) & 7 ? "dec" : "inc");
			currentCode = ParseModRM(currentCode, Instruction, operand1);

			break;
		}

		case 0xFF:
		{
			Instruction->Opcode = *currentCode;
			Instruction->wFlag = *currentCode & 1;

			currentCode++;
			switch((*currentCode >> 3) & 7)
			{
				case 0 : case 1: case 6:
				{
					sprintf(mnemonic, FFOpcodeMnemonic[(*currentCode >> 3) & 7]);
					currentCode = ParseModRM(currentCode, Instruction, operand1);

					break;
				}
				case 2 : 
				{
					sprintf(mnemonic, FFOpcodeMnemonic[(*currentCode >> 3) & 7]);
					currentCode = ParseModRM(currentCode, Instruction, operand1);

					// 增加是否跟踪打印的判断 [3/31/2012 xqx]
					if (dwPrintLevel == BLOCK_PRINT || dwPrintLevel == CALL_PRINT)
					{
						bNeedPrint = 1;
					}

					break;
				}
				case 4: 
				{
					sprintf(mnemonic, FFOpcodeMnemonic[(*currentCode >> 3) & 7]);
					currentCode = ParseModRM(currentCode, Instruction, operand1);

					// 增加是否跟踪打印的判断 [3/31/2012 xqx]
					if (dwPrintLevel == BLOCK_PRINT )
					{
						bNeedPrint = 1;
					}

					break;
				}
				case 3 : case 5:
				{
					sprintf(mnemonic, FFOpcodeMnemonic[(*currentCode >> 3) & 7]);
					currentCode = ParseModRM(currentCode, Instruction, operand1);
					sprintf(operand2, "far %s%s", Instruction->OperandPrefix >= 0 ? "dword " : "fword ",
						strstr(operand1, "ptr"));
					sprintf(operand1, "%s", operand2);
					*operand2 = '\0';

					// 增加是否跟踪打印的判断 [3/31/2012 xqx]
					if (dwPrintLevel == BLOCK_PRINT)
					{
						bNeedPrint = 1;
					}

					break;
				}
				default:
				{
					sprintf(mnemonic, "???");
					currentCode++;
				}
			}

			break;
		}

		default :
		{
			currentCode++;
			sprintf(mnemonic, "%s", "???");
		}
	}

	/* instruction str = [prefix] mnemonic [operand1,] [operand2,] [operand3] */

	sprintf(InstructionStr, "%s%s%s%s%s%s%s%s%s", 
		prefix, strlen(prefix) > 0 ? "\t" : "",
		mnemonic, "\t",
		operand1,
		strlen(operand2) > 0 ? ", " : "", operand2,
		strlen(operand3) > 0 ? ", " : "", operand3);
	
	return currentCode;
}

unsigned char *ParseModRM(unsigned char *Code, PINSTRUCTION Instruction, char *OperandRM)
{
	unsigned char *currentCode;
	char Mod, RM;
	char DisplacementStr[MAX_OPERAND_LEN] = "";
	char RMStr[MAX_OPERAND_LEN] = "";
	char SizeStr[MAX_OPERAND_LEN] = "";
	char AddressStr[MAX_OPERAND_LEN] = "";
	char SegmentPrefixStr[MAX_OPERAND_LEN] ="";

	currentCode = Code;
	Instruction->ModRM = *currentCode;
	
	/* get mod, reg/code, rm */

	Mod = (*currentCode >> 6) & 3;
	RM = (*currentCode) & 7;

	sprintf(SegmentPrefixStr, "%s%s", Instruction->SegmentPrefix >= 0 ? SegmentRegisters[Instruction->SegmentPrefix] : "", 
		Instruction->SegmentPrefix >= 0 ? ":" : "");
	sprintf(SizeStr, "%s", Instruction->wFlag ? (Instruction->OperandPrefix >= 0 ? "word ptr" : "dword ptr") : "byte ptr");
	sprintf(AddressStr, "%s", Instruction->AddressPrefix >=0 ? Address16[RM] : Register32[RM]);

	switch(Mod)
	{
		case 0 :		/* 00 */
		{
			if(RM != 4 && RM != 5 && RM != 6)			/* 100, 101,110 */
			{
				sprintf(RMStr, "%s", AddressStr);
				
				currentCode++;
			}
			else if(RM == 4)				/* 100 */
			{
				if(Instruction->AddressPrefix >= 0)			/* 16 bits mode */
				{
					sprintf(RMStr, "%s", Address16[RM]);

					currentCode++;
				}
				else														/*32 bits mode */
				{
					/* SIB */
					
					currentCode++;
					currentCode = ParseSIB(currentCode, Instruction, RMStr);
					currentCode++;
				}
			}
			else if(RM == 5)				/* 101 */
			{
				if(Instruction->AddressPrefix >= 0)			/* 16 bits mode */
				{
					sprintf(RMStr, "%s", Address16[RM]);

					currentCode++;
				}
				else														/*32 bits mode */
				{
					/* displacement 32 */

					currentCode++;
					Instruction->Displacement = *((unsigned int *)currentCode);
					
					sprintf(RMStr, "%X", *((unsigned int *)currentCode));
					currentCode += 4;
				}
			}
			else if(RM == 6)				/* 110 */ 
			{
				if(Instruction->AddressPrefix >= 0)			/* 16 bits mode */
				{
					/* displacement 16 */

					currentCode++;
					memcpy(&(Instruction->Displacement), currentCode, 2);
					
					sprintf(RMStr, "%X%X%X%X", (*(currentCode + 1) >> 4) & 0xF,
						(*(currentCode + 1) & 0xF), (*currentCode >> 4) & 0xF, (*currentCode) & 0xF);
					
					currentCode += 2;
				}
				else														/* 32 bits mode */
				{
					sprintf(RMStr, "%s", Register32[RM]);

					currentCode++;
				}
			}
			
			sprintf(OperandRM, "%s %s[%s]", SizeStr, SegmentPrefixStr, RMStr);
			
			break;
		}
		case 1 :		/* 01 */
		{
			
			if(RM != 4)
			{
				sprintf(RMStr, "%s", Register32[RM]);
			}
			else
			{
				/* SIB */
				
				if(Instruction->AddressPrefix >= 0)
				{
					sprintf(RMStr, "%s", Address16[RM]);
				}
				else
				{
					currentCode++;
					currentCode = ParseSIB(currentCode, Instruction, RMStr);
				}
			}
			
			/* displacement 8 */

			currentCode++;
			Instruction->Displacement = *currentCode;
			sprintf(DisplacementStr, "%s%X%X", (*currentCode >>7) & 1 ? " - " : " + ", (*currentCode >> 7) & 1 ? (~(*currentCode) + 1) >> 4 & 0xF : (*currentCode >> 4) & 0xF, 
				(*currentCode >> 7) & 1 ? (~(*currentCode) + 1) & 0xF : (*currentCode) & 0xF);
			currentCode++;

			sprintf(OperandRM, "%s %s[%s%s]", SizeStr, SegmentPrefixStr, RMStr, DisplacementStr);

			break;
		}
		case 2 :		/* 10 */
		{
			if(RM != 4)
			{
				sprintf(RMStr, "%s", Register32[RM]);
			}
			else
			{
				/* SIB */
				
				if(Instruction->AddressPrefix >= 0)
				{
					sprintf(RMStr, "%s", Address16[RM]);
				}
				else
				{
					currentCode++;
					currentCode = ParseSIB(currentCode, Instruction, RMStr);
				}
			}
			
			/* displacement 32 */
			
			currentCode++;
			Instruction->Displacement = *((int *)currentCode);
			sprintf(DisplacementStr, " %c %X", ((*((int *)currentCode)) >> 31) & 1 ? '-' : '+', ((*((int *)currentCode)) >> 31) & 1?  ~(*((int *)currentCode)) + 1 : *((int *)currentCode));
			currentCode += 4;

			sprintf(OperandRM, "%s %s[%s%s]", SizeStr, SegmentPrefixStr, RMStr, DisplacementStr);

			break;
		}
		case 3 :		/* 11 */
		{
			sprintf(OperandRM, "%s", Instruction->wFlag ? (Instruction->OperandPrefix >= 0 ? Register16[RM] : Register32[RM]) 
				 : Register8[RM]);

			currentCode++;

			break;
		}
	}

	return currentCode;
}

unsigned char *ParseRegModRM(unsigned char *Code, PINSTRUCTION Instruction, char *Operand1, char *Operand2)
{
	unsigned char *currentCode;
	char RegOpcode;

	currentCode = Code;

	RegOpcode = (*currentCode >> 3) & 7;
	
	currentCode = ParseModRM(currentCode, Instruction, Instruction->dFlag ? Operand2 : Operand1);
	sprintf(Instruction->dFlag ? Operand1 : Operand2, "%s", Instruction->wFlag ? (Instruction->OperandPrefix >= 0 ? Register16[RegOpcode] : Register32[RegOpcode]) 
				 : Register8[RegOpcode]);

	return currentCode;
}

unsigned char *ParseImmediate(unsigned char *Code, PINSTRUCTION Instruction, char *OperandImmediate)
{
	unsigned char *currentCode;
	
	currentCode = Code;

	if(!Instruction->sFlag)
	{
		if(Instruction->OperandPrefix >= 0)
		{
			/* 16 bits immediate value */
			
			memcpy(&(Instruction->Immediate), currentCode, 2);
			sprintf(OperandImmediate, "%X%X%X%X", (*(currentCode + 1) >> 4) & 0xF, 
				(*(currentCode + 1) & 0xF), (*currentCode >> 4) & 0xF, (*currentCode) & 0xF);

			currentCode += 2;
		}
		else
		{
			/* 32 bits immediate value */

			Instruction->Immediate = *((unsigned int *)currentCode);
			sprintf(OperandImmediate, "%X", *((unsigned int *)currentCode));

			currentCode += 4;
		}
	}
	else
	{
		/* 8 bits immediate value */

		Instruction->Immediate = *currentCode;
		sprintf(OperandImmediate, "%X%X", (*currentCode >> 4) & 0xF, (*currentCode) & 0xF);
		
		currentCode++;
	}

	return currentCode;
}

unsigned char *ParseSIB(unsigned char *Code, PINSTRUCTION Instruction, char *SIBStr)
{
	char *currentCode;
	char Mod;
	char Scale, Index, Base;
	char BaseStr[MAX_OPERAND_LEN] = "";
	char ScaledIndexStr[MAX_OPERAND_LEN] = "";
	char DisplacementStr[MAX_OPERAND_LEN] = "";
	
	currentCode = (char*)Code;
	Instruction->SIB = *currentCode;
	
	Mod = ((Instruction->ModRM) >> 6) & 3;
	Scale = (*currentCode >> 6) & 3;
	Index = (*currentCode >> 3) & 7;
	Base = (*currentCode) & 7;

	/* base */

	sprintf(BaseStr, "%s", (Base ==5 && Mod == 0) ?  "" : Register32[Base]);

	/* when Mod == 00b and Base = 101b, there is a special displacement 32 */

	if(Base == 5 && Mod == 0)
	{
		currentCode ++;
		Instruction->Displacement = *((long *)currentCode);

		sprintf(DisplacementStr, "%X", *((int *)currentCode));
		currentCode += 3;
	}

	/* scaled index */
	
	Index == 4 ? sprintf(ScaledIndexStr, "") : 
		(Scale ? sprintf(ScaledIndexStr, "%s * %d", Register32[Index], 1 << Scale) : 
		sprintf(ScaledIndexStr, "%s", Register32[Index]));

	/* merge them into SIB */

	sprintf(SIBStr, "%s%s%s%s%s", BaseStr,
		(strlen(BaseStr) > 0 && strlen(ScaledIndexStr) > 0) ? " + " : "",
		ScaledIndexStr,
		(strlen(BaseStr) > 0 && strlen(ScaledIndexStr) > 0 && strlen(DisplacementStr) > 0) ? " + " : "",
		DisplacementStr);

	return (unsigned char*)currentCode;
}

