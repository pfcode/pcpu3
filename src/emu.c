#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "emu-dump.h"
#include "rom-default.h"

#define DEBUG_OUTPUT 1
#define VERSION_STRING "0.1"

/* Registers */
uint32	A1;
uint16	B1;
uint16	B2;
uint8	C1;
uint8	C2;

uint8	CF;
uint8	NF;
uint16	EX;

/* Random Access Memory */
uint8	*RAM;
int 	RAM_SIZE;

/* Internal flags */
char halted = 0;

/* Interrupt table */
t_ptr interrupts[256];

t_stack getStack(){
	t_stack tmp;
	tmp.A1 = A1;
	tmp.B1 = B1;
	tmp.B2 = B2;
	tmp.C1 = C1;
	tmp.C2 = C2;
	tmp.CF = CF;
	tmp.NF = NF;
	tmp.EX = EX;
	return tmp;
}

void setStack(t_stack tmp){
	A1 = tmp.A1;
	B1 = tmp.B1;
	B2 = tmp.B2;
	C1 = tmp.C1;
	C2 = tmp.C2;
	CF = tmp.CF;
	NF = tmp.NF;
	EX = tmp.EX;
}

int prepare(int memsize){
	// Allocate memory for RAM
	RAM 	= (uint8 *) malloc(sizeof(uint8) * memsize);
	RAM_SIZE	= memsize;

	// Reset register values
	A1	= 0;
	B1	= 0;
	B2	= 0;
	C1	= 0;
	C2	= 0;

	CF	= 0;
	NF	= 0;
	EX	= 0;

	// Reset interrupts table
	int i;
	for(i = 0; i < 256; i++) interrupts[i];
}

void execute();

void dumpRegs(){
	printf("A1:%08X\n", A1);
	printf("B1:%04X\tB2:%04X\n", B1, B2);
	printf("C1:%02X\tC2:%02X\n", C1, C2);
	printf("CF:%01X\tNF:%02X\tEX:%04X\n", CF, NF, EX);
}

void dumpRegsInline(){
	printf("[A1:%08X B1:%04X B2:%04X C1:%02X C2:%02X CF:%01X NF:%02X EX:%04X]", A1, B1, B2, C1, C2, CF, NF, EX);
}

void loadROM(){
	int i;
	for(i = 0; i < ROM_SIZE && i < RAM_SIZE; i++) RAM[i] = ROM[i];
}

void cleanup(){
	free(RAM);
}

void di_out(uint8 port, uint32 value){
	switch(port){
		default: return;
		case 0x00: halted = 1; return;
		case 0x01:
			A1 	= 0;
			B1	= 0;
			B2	= 0;
			C1	= 0;
			C2	= 0;
			CF	= 0;
			NF	= 0;
			EX	= 0;
			return;
		case 0x20:
			putchar((uint8) value);
			return;
	}
}

uint32 di_in(uint8 port){
	return 0;
}

uint8 getRAM8(int offset){
	if(offset < 0 || offset > RAM_SIZE){
		printf("Critical fault: Program tried to read memory out of range.\n");
		return 0;
	}
	return RAM[offset];
}

uint16 getRAM16(int offset){
	if(offset < 0 || offset > RAM_SIZE){
		printf("Critical fault: Program tried to read memory out of range.\n");
		return 0;
	}
	return (uint16) ((RAM[offset] << 8) + RAM[offset + 1]);
}

uint32 getRAM32(int offset){
	return (uint32) ((RAM[offset] << 24) + (RAM[offset + 1] << 16) + (RAM[offset + 2] << 8) + RAM[offset + 3]);
}

void setRAM8(int offset, uint32 value){
	if(offset < 0 || offset > RAM_SIZE){
		printf("Critical fault: Program tried to write memory out of range.\n");
		return;
	}
	RAM[offset] = (uint8) value;
}

void setRAM16(int offset, uint32 value){
	if(offset < 0 || offset + 1> RAM_SIZE){
		printf("Critical fault: Program tried to write memory out of range.\n");
		return;
	}
	RAM[offset + 1] = (uint8) value;
	RAM[offset] = (uint8) (value >> 8);
}

void setRAM32(int offset, uint32 value){
	if(offset < 0 || offset + 3> RAM_SIZE){
		printf("Critical fault: Program tried to write memory out of range.\n");
		return;
	}
	RAM[offset + 3] = (uint8) value;
	RAM[offset + 2] = (uint8) (value >> 8);
	RAM[offset + 1] = (uint8) (value >> 16);
	RAM[offset] = (uint8) (value >> 24);
}

uint32 getReg(int regid){
	switch(regid){
		default: return 0;
		case REG_A1: return A1;
		case REG_B1: return B1;
		case REG_B2: return B2;
		case REG_C1: return C1;
		case REG_C2: return C2;
		case REG_CF: return CF;
		case REG_NF: return NF;
		case REG_EX: return EX;
	}
}

void setReg(int regid, uint32 value){
	switch(regid){
		default: return;
		case REG_A1: A1 = value; return;
		case REG_B1: B1 = (uint16) value; return;
		case REG_B2: B2 = (uint16) value; return;
		case REG_C1: C1 = (uint8) value; return;
		case REG_C2: C2 = (uint8) value; return;
//		case REG_CF: CF = (uint8) value; return;
		case REG_NF: NF = (uint8) value; return;
//		case REG_EX: EX = (uint16) value; return;
	}
}

// GET.8 instruction
void i_get8(){
	uint8 reg	= getRAM8(EX + 1); 	// First arg: 8-bit
	t_ptr pointer	= getRAM16(EX + 2);	// Second arg: 16-bit
	setReg(reg, getRAM8(pointer));
	EX += 3;
}

// GET.16 instruction
void i_get16(){
	uint8 reg	= getRAM8(EX + 1); 	// First arg: 8-bit
	t_ptr pointer	= getRAM16(EX + 2);	// Second arg: 16-bit
	setReg(reg, getRAM16(pointer));
	EX += 3;
}

// GET.32 instruction
void i_get32(){
	uint8 reg	= getRAM8(EX + 1); 	// First arg: 8-bit
	t_ptr pointer	= getRAM16(EX + 2);	// Second arg: 16-bit
	setReg(reg, getRAM32(pointer));
	EX += 3;
}

// IGET.8 instruction
void i_iget8(){
	uint8 reg	= getRAM8(EX + 1); 	// First arg: 8-bit
	t_ptr pointer	= getReg(getRAM8(EX + 2));	// Second arg: 8-bit
	setReg(reg, getRAM8(pointer));
	EX += 2;
}

// IGET.16 instruction
void i_iget16(){
	uint8 reg	= getRAM8(EX + 1); 	// First arg: 8-bit
	t_ptr pointer	= getReg(getRAM8(EX + 2));	// Second arg: 8-bit
	setReg(reg, getRAM16(pointer));
	EX += 2;
}

// IGET.32 instruction
void i_iget32(){
	uint8 reg	= getRAM8(EX + 1); 	// First arg: 8-bit
	t_ptr pointer	= getReg(getRAM8(EX + 2));	// Second arg: 8-bit
	setReg(reg, getRAM32(pointer));
	EX += 2;
}

// SAVE.8 instruction
void i_save8(){
	t_ptr pointer	= getRAM16(EX + 1);	// First arg: 16-bit
	uint8 reg	= getRAM8(EX + 3); 	// Second arg: 8-bit
	setRAM8(pointer, getReg(reg));
	EX += 3;
}

// SAVE.16 instruction
void i_save16(){
	t_ptr pointer	= getRAM16(EX + 1);	// First arg: 16-bit
	uint8 reg	= getRAM8(EX + 3); 	// Second arg: 8-bit
	setRAM16(pointer, getReg(reg));
	EX += 3;
}

// SAVE.32 instruction
void i_save32(){
	t_ptr pointer	= getRAM16(EX + 1);	// First arg: 16-bit
	uint8 reg	= getRAM8(EX + 3); 	// Second arg: 8-bit
	setRAM32(pointer, getReg(reg));
	EX += 3;
}

// ISAVE.8 instruction
void i_isave8(){
	t_ptr pointer	= getReg(getRAM8(EX + 1));	// First arg: 8-bit
	uint8 reg	= getRAM8(EX + 2); 	// Second arg: 8-bit
	setRAM8(pointer, getReg(reg));
	EX += 2;
}

// ISAVE.16 instruction
void i_isave16(){
	t_ptr pointer	= getReg(getRAM8(EX + 1));	// First arg: 8-bit
	uint8 reg	= getRAM8(EX + 2); 	// Second arg: 8-bit
	setRAM16(pointer, getReg(reg));
	EX += 2;
}

// ISAVE.32 instruction
void i_isave32(){
	t_ptr pointer	= getReg(getRAM16(EX + 1));	// First arg: 8-bit
	uint8 reg	= getRAM8(EX + 2); 	// Second arg: 8-bit
	setRAM32(pointer, getReg(reg));
	EX += 2;
}

// MOV instruction
void i_mov(){
	uint8 reg_out	= getRAM8(EX + 1);	// First arg: 8-bit
	uint8 reg_in	= getRAM8(EX + 2);	// Second arg: 8-bit
	setReg(reg_out, getReg(reg_in));
	EX += 2;
}

// NEG instruction
void i_neg(){
	uint8 reg	= getRAM8(EX + 1);
	setReg(reg, ~getReg(reg));
	EX += 1;
}

// SHL instruction
void i_shl(){
	uint8 reg	= getRAM8(EX + 1);
	setReg(reg, getReg(reg) << 1);
	EX += 1;
}

// SHR instruction
void i_shr(){
	uint8 reg	= getRAM8(EX + 1);
	setReg(reg, getReg(reg) >> 1);
	EX += 1;
}

// ADD instruction
void i_add(){
	uint8 reg_out	= getRAM8(EX + 1);
	uint8 reg_in	= getRAM8(EX + 2);
	setReg(reg_out, getReg(reg_out) + getReg(reg_in));
	EX += 2;
}

// SUB instruction
void i_sub(){
	uint8 reg_out	= getRAM8(EX + 1);
	uint8 reg_in	= getRAM8(EX + 2);
	setReg(reg_out, getReg(reg_out) - getReg(reg_in));
	EX += 2;
}

// MUL instruction
void i_mul(){
	uint8 reg_out	= getRAM8(EX + 1);
	uint8 reg_in	= getRAM8(EX + 2);
	setReg(reg_out, getReg(reg_out) * getReg(reg_in));
	EX += 2;
}

// DIV instruction
void i_div(){
	uint8 reg_out	= getRAM8(EX + 1);
	uint8 reg_in	= getRAM8(EX + 2);
	setReg(reg_out, getReg(reg_out) / getReg(reg_in));
	EX += 2;
}

// MOD instruction
void i_mod(){
	uint8 reg_out	= getRAM8(EX + 1);
	uint8 reg_in	= getRAM8(EX + 2);
	setReg(reg_out, getReg(reg_out) % getReg(reg_in));
	EX += 2;
}

// AND instruction
void i_and(){
	uint8 reg_out	= getRAM8(EX + 1);
	uint8 reg_in	= getRAM8(EX + 2);
	setReg(reg_out, getReg(reg_out) & getReg(reg_in));
	EX += 2;
}

// OR instruction
void i_or(){
	uint8 reg_out	= getRAM8(EX + 1);
	uint8 reg_in	= getRAM8(EX + 2);
	setReg(reg_out, getReg(reg_out) | getReg(reg_in));
	EX += 2;
}

// XOR instruction
void i_xor(){
	uint8 reg_out	= getRAM8(EX + 1);
	uint8 reg_in	= getRAM8(EX + 2);
	setReg(reg_out, getReg(reg_out) ^ getReg(reg_in));
	EX += 2;
}

// OUT instruction
void i_out(){
	uint8 port	= getRAM8(EX + 1);
	uint8 reg	= getRAM8(EX + 2);
	di_out(port, getReg(reg));
	EX += 2;
}

// IN instruction
void i_in(){
	uint8 reg	= getRAM8(EX + 1);
	uint8 port	= getRAM8(EX + 2);
	setReg(reg, di_in(port));
	EX += 2;
}

// CMP instruction
void i_cmp(){
	uint32 reg1	= getReg(getRAM8(EX + 1));
	uint32 reg2	= getReg(getRAM8(EX + 2));
	if(reg1 == reg2) CF = 0;
	else if(reg1 > reg2) CF = 1;
	else CF = 2;

	EX += 2;
}

// JMP instruction
void i_jmp(){
	t_ptr pointer	= getRAM16(EX + 1);
	EX = pointer - 1;
}

// JE instruction
void i_je(){
	if(CF == 0) i_jmp();
	else EX += 2;
}

// JNE instruction
void i_jne(){
	if(CF != 0) i_jmp();
	else EX += 2;
}

// JG instruction
void i_jg(){
	if(CF == 1) i_jmp();
	else EX += 2;
}

// JL instruction
void i_jl(){
	if(CF == 2) i_jmp();
	else EX += 2;
}

// INT instruction
void i_int(){
	int i;
	t_stack stack = getStack();
	EX = interrupts[NF];
	execute();
	setStack(stack);
	halted = false;
}

void execute(){
	while(!halted && EX < RAM_SIZE){
		if(DEBUG_OUTPUT){
			dumpInstruction(&RAM[EX]);
			printf("\t");
			dumpRegsInline();
		}
		switch(RAM[EX]){
			default: break;
			case NUL: break;
			case GET8: i_get8(); break;
			case GET16: i_get16(); break;
			case GET32: i_get32(); break;
			case IGET8: i_iget8(); break;
			case IGET16: i_iget16(); break;
			case IGET32: i_iget32(); break;
			case SAVE8: i_save8(); break;
			case SAVE16: i_save16(); break;
			case SAVE32: i_save32(); break;
			case ISAVE8: i_isave8(); break;
			case ISAVE16: i_isave16(); break;
			case ISAVE32: i_isave32(); break;

			case MOV: i_mov(); break;

			case NEG: i_neg(); break;
			case SHL: i_shl(); break;
			case SHR: i_shr(); break;

			case ADD: i_add(); break;
			case SUB: i_sub(); break;
			case MUL: i_mul(); break;
			case DIV: i_div(); break;
			case MOD: i_mod(); break;

			case AND: i_and(); break;
			case OR: i_or(); break;
			case XOR: i_xor(); break;

			case OUT: i_out(); break;
			case IN: i_in(); break;

			case CMP: i_cmp(); break;
			case JMP: i_jmp(); break;
			case JE: i_je(); break;
			case JNE: i_jne(); break;
			case JG: i_jg(); break;
			case JL: i_jl(); break;

			case INT: i_int(); break;
		}
		EX++;
		if(DEBUG_OUTPUT) getchar();
	}
}

int main(int argc, char *argv[]){
	uint16 pre_mem = 1024*1024;
	t_ptr pre_ex = 0x0000;
	FILE *romfile = NULL;
	char rompath[255];
	uint8 pre_rom[1024*1024];
	uint16 pre_romsize;
	char rom_select = 0;
	if(argc <= 1){
		printf("PCPU-v3 emulator\n");
		printf("Usage: %s <options> romfile.bin\n", argv[0]);
		printf("Available options:\n");
		printf(" --ex pointer\t\tSet EX (EXecution pointer) to start executing from specified memory pointer. Format is: 0x12FE\n");
		printf(" --mem bytes \t\tSet RAM size. Default: 1048576 (1MB).\n");
		printf(" --version   \t\tShow version of this emulator.\n");
		return 0;
	} else{
		for(int i = 1; i < argc; i++){
			if(strncmp(argv[i], "--ex", strlen("--ex")) == 0){
				if(sscanf(argv[i+1], "%x", &pre_ex) > 0){
					i++;
				}
			} else if(strncmp(argv[i], "--mem", strlen("--mem")) == 0){
				if(sscanf(argv[i+1], "%d", &pre_mem) > 0){
					i++;
				}
			} else if(strncmp(argv[i], "--version", strlen("--version")) == 0){
				printf("PCPU-v3 emulator. Author: pfcode. Version: %s\n", VERSION_STRING);
			} else{
				if(sscanf(argv[i], "%s", &rompath) > 0){
					romfile = fopen(rompath, "rb+");
					if(!romfile){
						printf("ROM file: %s is not found.\n", rompath);
						return 1;
					}
					int c = fgetc(romfile);
					int j = 0;
					while(c != EOF){
						pre_rom[j] = c;
						j++;
						c = fgetc(romfile);
					}
					pre_romsize = j;
					rom_select = 1;
				}
			}
		}
	}
	printf("Attaching %d bytes of RAM memory\n", pre_mem);
	prepare(pre_mem); /* 1MB RAM */
	EX = pre_ex;
	printf("Loading ROM on 0x0000\n");
	if(rom_select == 0) loadROM();
	else{
		for(int i = 0; i < pre_romsize; i++){
			RAM[i] = pre_rom[i];
		}
	}
	printf("Starting execution from 0x%04X\n", EX);
	execute();
	printf("The execution has stopped.\n");
	if(DEBUG_OUTPUT) dumpRegs();
	cleanup();
}
