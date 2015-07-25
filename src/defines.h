#pragma once

#define NUL		0

// Memory & registers
#define GET8		0x01
#define GET16		0x02
#define GET32		0x03
#define IGET8		0x04
#define IGET16		0x05
#define IGET32		0x06

#define SAVE8		0x07
#define SAVE16		0x08
#define SAVE32		0x09
#define ISAVE8		0x0a
#define ISAVE16		0x0b
#define ISAVE32		0x0c

#define MOV		0x0d

// Bitwise
#define NEG		0x0e
#define SHL		0x0f
#define SHR		0x10

// Math
#define ADD		0x11
#define SUB		0x12
#define MUL		0x13
#define DIV		0x14
#define MOD		0x15

// Logic
#define AND		0x16
#define OR		0x17
#define XOR		0x18

// I/O
#define OUT		0x19
#define IN		0x1a

// Comparation
#define CMP		0x1b
#define JMP		0x1c
#define JE		0x1d
#define JNE		0x1e
#define JG		0x1f
#define JL		0x20

// Interrupt
#define INT		0x21

// Register IDs
#define REG_A1		0x01
#define REG_B1		0x02
#define REG_B2		0x03
#define REG_C1		0x04
#define REG_C2		0x05
#define REG_CF		0x06
#define REG_NF		0x07
#define REG_EX		0x08

/* Type definitions */
typedef unsigned char uint8;
typedef unsigned int uint16;
typedef unsigned long uint32;
typedef uint16 t_ptr;

typedef struct{
	uint32 A1;
	uint16	B1;
	uint16	B2;
	uint8	C1;
	uint8	C2;

	uint8	CF;
	uint8	NF;
	uint16	EX;
} t_stack;
