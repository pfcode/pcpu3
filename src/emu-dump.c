#include <stdio.h>
#include <stdlib.h>

#include "defines.h"

const char *dumpRegister(int regid){
	static char tmp[16];
	switch(regid){
		default:
			sprintf(tmp, "%d", regid);
			return tmp;
		case REG_A1: return "A1";
		case REG_B1: return "B1";
		case REG_B2: return "B2";
		case REG_C1: return "C1";
		case REG_C2: return "C2";
		case REG_CF: return "CF";
		case REG_NF: return "NF";
		case REG_EX: return "EX";
	}
}

void dumpInstruction(uint8 *mem){
	switch(mem[0]){
		default: printf("[ UNKNOWN0x%X ]", mem[0]); break;
		case NUL: printf("[ NUL ]", mem[0]); break;
		case GET8: printf("[ GET8 %s 0x%04X ]", dumpRegister(mem[1]), (mem[2] << 8) + mem[3]); break;
		case GET16: printf("[ GET16 %s 0x%04X ]", dumpRegister(mem[1]), (mem[2] << 8) + mem[3]); break;
		case GET32: printf("[ GET32 %s 0x%04X ]", dumpRegister(mem[1]), (mem[2] << 8) + mem[3]); break;
		case IGET8: printf("[ IGET8 %s 0x%04X ]", dumpRegister(mem[1]), (mem[2] << 8) + mem[3]); break;
		case IGET16: printf("[ IGET16 %s 0x%04X ]", dumpRegister(mem[1]), (mem[2] << 8) + mem[3]); break;
		case IGET32: printf("[ IGET32 %s 0x%04X ]", dumpRegister(mem[1]), (mem[2] << 8) + mem[3]); break;
		case SAVE8: printf("[ SAVE8 0x%04X %s ]", (mem[1] << 8) + mem[2], dumpRegister(mem[3])); break;
		case SAVE16: printf("[ SAVE16 0x%04X %s ]", (mem[1] << 8) + mem[2], dumpRegister(mem[3])); break;
		case SAVE32: printf("[ SAVE32 0x%04X %s ]", (mem[1] << 8) + mem[2], dumpRegister(mem[3])); break;
		case ISAVE8: printf("[ ISAVE8 0x%04X %s ]", (mem[1] << 8) + mem[2], dumpRegister(mem[3])); break;
		case ISAVE16: printf("[ ISAVE16 0x%04X %s ]", (mem[1] << 8) + mem[2], dumpRegister(mem[3])); break;
		case ISAVE32: printf("[ ISAVE32 0x%04X %s ]", (mem[1] << 8) + mem[2], dumpRegister(mem[3])); break;

		case MOV: printf("[ MOV %s %s ]", dumpRegister(mem[1]), dumpRegister(mem[2])); break;

		case NEG: printf("[ NEG %s ]", dumpRegister(mem[1])); break;
		case SHL: printf("[ SHL %s ]", dumpRegister(mem[1])); break;
		case SHR: printf("[ SHR %s ]", dumpRegister(mem[1])); break;

		case ADD: printf("[ ADD %s %s ]", dumpRegister(mem[1]), dumpRegister(mem[2])); break;
		case SUB: printf("[ SUB %s %s ]", dumpRegister(mem[1]), dumpRegister(mem[2])); break;
		case MUL: printf("[ MUL %s %s ]", dumpRegister(mem[1]), dumpRegister(mem[2])); break;
		case DIV: printf("[ DIV %s %s ]", dumpRegister(mem[1]), dumpRegister(mem[2])); break;
		case MOD: printf("[ MOD %s %s ]", dumpRegister(mem[1]), dumpRegister(mem[2])); break;
		case AND: printf("[ AND %s %s ]", dumpRegister(mem[1]), dumpRegister(mem[2])); break;
		case OR: printf("[ OR %s %s ]", dumpRegister(mem[1]), dumpRegister(mem[2])); break;
		case XOR: printf("[ XOR %s %s ]", dumpRegister(mem[1]), dumpRegister(mem[2])); break;

		case OUT: printf("[ OUT 0x%02X %s ]", mem[1], dumpRegister(mem[2])); break;
		case IN: printf("[ IN %s 0x%02X ]", dumpRegister(mem[1]), mem[2]); break;

		case CMP: printf("[ CMP %s %s ]", dumpRegister(mem[1]), dumpRegister(mem[2])); break;
		case JMP: printf("[ JMP 0x%04X ]", (mem[1] << 8) + mem[2]); break;
		case JE: printf("[ JE 0x%04X ]", (mem[1] << 8) + mem[2]); break;
		case JNE: printf("[ JNE 0x%04X ]", (mem[1] << 8) + mem[2]); break;
		case JG: printf("[ JL 0x%04X ]", (mem[1] << 8) + mem[2]); break;
		case JL: printf("[ JG 0x%04X ]", (mem[1] << 8) + mem[2]); break;

		case INT: printf("[ INT ]", (mem[1] << 8) + mem[2]); break;
	}
}
