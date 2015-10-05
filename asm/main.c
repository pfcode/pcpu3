#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../src/defines.h"

// Output binary
uint8 *output;

// Source file
char **sourceCode;
long codeLines;

/*
	VARIABLE is always considered as a pointer variable
	CONSTANT is always considered as a pointer to the constant value in the stack
*/

#define WORD_SPECIAL 1		// e.g. LABEL
#define WORD_LABELNAME 2	// e.g. main
#define WORD_OPCODE 3		// e.g. GET8
#define WORD_REGISTER 4		// e.g. AX
#define WORD_POINTER 5		// e.g. 0xA230		When pointer's strlen > 0, then it is a variable
#define WORD_VARNAME 6		// e.g. variable
#define WORD_VALUE 7		// e.g. 0xAB		Always 8-bit
#define WORD_VALUE2 8		// e.g. 0xABCD
#define WORD_VALUE4 9		// e.g. 0xABCDABCD

#define SYMBOL_DB 1
#define SYMBOL_DW 2
#define SYMBOL_DL 4

typedef struct{
	char type;
	char str[256];
	int value;
} t_word;

typedef struct{
	int wordCount;
	t_word *words;
} t_wordline;

t_wordline *sourceLines;

long executive_size = 0;
long stack_size = 0;

typedef struct{
	int length;
	uint32 value;
	char name[256];
} t_datafield;

t_datafield *stack;
long stackItems;

char * strtolower(char *string){
	char *cp;

	cp = string;
	while(*cp) {
		*cp = tolower(*cp);
		cp++;
	}
	return(string);
}

uint8 parseSymbolCode(char *s){
	s = strtolower(s);
	if(strcmp(s, "db") == 0) return SYMBOL_DB;
	if(strcmp(s, "dw") == 0) return SYMBOL_DW;
	if(strcmp(s, "dl") == 0) return SYMBOL_DL;

	printf("[Syntax] ParseSymbolCode: Invalid assembler symbol: %s.\n", s);
	exit(1);

	return 0;
}

uint8 parseRegisterCode(char *s){
	s = strtolower(s);
	if(strcmp(s, "a1") == 0) return REG_A1;
	if(strcmp(s, "b1") == 0) return REG_B1;
	if(strcmp(s, "b2") == 0) return REG_B2;
	if(strcmp(s, "c1") == 0) return REG_C1;
	if(strcmp(s, "c2") == 0) return REG_C2;
	if(strcmp(s, "cf") == 0) return REG_CF;
	if(strcmp(s, "nf") == 0) return REG_NF;
	if(strcmp(s, "ex") == 0) return REG_EX;

	printf("[Syntax] ParseRegisterCode: Invalid register symbol: %s.\n", s);
	exit(1);

	return 0;
}

uint8 parseInstructionCode(char *s){
	s = strtolower(s);
	if(strcmp(s, "get8") == 0) return GET8;
	if(strcmp(s, "get16") == 0) return GET16;
	if(strcmp(s, "get32") == 0) return GET32;
	if(strcmp(s, "save8") == 0) return SAVE8;
	if(strcmp(s, "save16") == 0) return SAVE16;
	if(strcmp(s, "save32") == 0) return SAVE32;
	if(strcmp(s, "iget8") == 0) return IGET8;
	if(strcmp(s, "iget16") == 0) return IGET16;
	if(strcmp(s, "iget32") == 0) return IGET32;
	if(strcmp(s, "isave8") == 0) return ISAVE8;
	if(strcmp(s, "isave16") == 0) return ISAVE16;
	if(strcmp(s, "isave32") == 0) return ISAVE32;
	if(strcmp(s, "mov") == 0) return MOV;
	if(strcmp(s, "neg") == 0) return NEG;
	if(strcmp(s, "shl") == 0) return SHL;
	if(strcmp(s, "shr") == 0) return SHR;
	if(strcmp(s, "add") == 0) return ADD;
	if(strcmp(s, "sub") == 0) return SUB;
	if(strcmp(s, "mul") == 0) return MUL;
	if(strcmp(s, "div") == 0) return DIV;
	if(strcmp(s, "mod") == 0) return MOD;
	if(strcmp(s, "and") == 0) return AND;
	if(strcmp(s, "or") == 0) return OR;
	if(strcmp(s, "xor") == 0) return XOR;
	if(strcmp(s, "out") == 0) return OUT;
	if(strcmp(s, "in") == 0) return IN;
	if(strcmp(s, "cmp") == 0) return CMP;
	if(strcmp(s, "jmp") == 0) return JMP;
	if(strcmp(s, "je") == 0) return JE;
	if(strcmp(s, "jne") == 0) return JNE;
	if(strcmp(s, "jg") == 0) return JG;
	if(strcmp(s, "jl") == 0) return JL;
	if(strcmp(s, "int") == 0) return INT;

	printf("[Syntax] ParseInstructionCode: Invalid opcode symbol: %s.\n", s);
	exit(1);

	return 0;
}

void getWords(){
	char *wordzero;
	char s1[255], s2[255], s3[255];		// string buffers
	int i1, i2, i3; 			// int buffers
	char f1, f2, f3;			// flags
	wordzero = (char *) malloc(sizeof(char) * 256);
	sourceLines = (t_wordline *) malloc(sizeof(t_wordline) * codeLines);
	for(int i = 0; i < codeLines; i++){
		f1 = 0; f2 = 0; f3 = 0;
		sscanf(sourceCode[i], "%s", wordzero);
		wordzero = strtolower(wordzero);
		if(strcmp("get8", wordzero) == 0
		|| strcmp("get16", wordzero) == 0
		|| strcmp("get32", wordzero) == 0
		|| strcmp("iget8", wordzero) == 0
		|| strcmp("iget16", wordzero) == 0
		|| strcmp("iget32", wordzero) == 0){
			// WORD_OPCODE WORD_REGISTER WORD_POINTER
			if(sscanf(sourceCode[i], "%s %s 0x%x", &s1, &s2, &i1) != 3){
				f1 = 1; // Pointer is a variable
				if(sscanf(sourceCode[i], "%s %s %s", &s1, &s2, &s3) != 3 || !isalpha(s3[0])){
					printf("[Syntax] GetWords: Incorrect %s usage in line: \n\t%s\n", wordzero, sourceCode[i]);
					exit(1);
				}
			}
			sourceLines[i].wordCount = 3;
			sourceLines[i].words = (t_word *) malloc(sizeof(t_word) * sourceLines[i].wordCount);

			sourceLines[i].words[0].type = WORD_OPCODE;
			strcpy(sourceLines[i].words[0].str, s1);
			sourceLines[i].words[0].value = parseInstructionCode(s1);

			sourceLines[i].words[1].type = WORD_REGISTER;
			strcpy(sourceLines[i].words[1].str, s2);
			sourceLines[i].words[1].value = parseRegisterCode(s2);

			sourceLines[i].words[2].type = WORD_POINTER;
			if(f1 == 0){
				sourceLines[i].words[2].value = i1;
				sourceLines[i].words[2].str[0] = '\0';
				if(i1 > 0xFFFF) printf("[Warning] GetWords: Pointer seems to be out of range: 0x%X (PCPUv3 supports 16-bit addressing) in line: \n\t%s\n", i1, sourceCode[i]);
			} else{
				strcpy(sourceLines[i].words[2].str, s3);
			}
		} else if(strcmp("save8", wordzero) == 0
		|| strcmp("save16", wordzero) == 0
		|| strcmp("save32", wordzero) == 0
		|| strcmp("isave8", wordzero) == 0
		|| strcmp("isave16", wordzero) == 0
		|| strcmp("isave32", wordzero) == 0){
			// WORD_OPCODE WORD_POINTER WORD_REGISTER
			if(sscanf(sourceCode[i], "%s 0x%x %s", &s1, &i1, &s2) != 3){
				f1 = 1; // Pointer is a variable
				if(sscanf(sourceCode[i], "%s %s %s", &s1, &s3, &s2) != 3 || !isalpha(s3[0])){
					printf("[Syntax] GetWords: Incorrect %s usage in line: \n\t%s\n", wordzero, sourceCode[i]);
					exit(1);
				}
			}
			sourceLines[i].wordCount = 3;
			sourceLines[i].words = (t_word *) malloc(sizeof(t_word) * sourceLines[i].wordCount);

			sourceLines[i].words[0].type = WORD_OPCODE;
			strcpy(sourceLines[i].words[0].str, s1);
			sourceLines[i].words[0].value = parseInstructionCode(s1);

			sourceLines[i].words[1].type = WORD_POINTER;
			if(f1 == 0){
				sourceLines[i].words[1].value = i1;
				sourceLines[i].words[1].str[0] = '\0';
				if(i1 > 0xFFFF) printf("[Warning] GetWords: Pointer seems to be out of range: 0x%X (PCPUv3 supports 16-bit addressing) in line: \n\t%s\n", i1, sourceCode[i]);
			} else{
				strcpy(sourceLines[i].words[1].str, s3);
			}

			sourceLines[i].words[2].type = WORD_REGISTER;
			strcpy(sourceLines[i].words[2].str, s2);
			sourceLines[i].words[2].value = parseRegisterCode(s2);
		} else if(strcmp("mov", wordzero) == 0
		|| strcmp("add", wordzero) == 0
		|| strcmp("sub", wordzero) == 0
		|| strcmp("mul", wordzero) == 0
		|| strcmp("div", wordzero) == 0
		|| strcmp("mod", wordzero) == 0
		|| strcmp("and", wordzero) == 0
		|| strcmp("or", wordzero) == 0
		|| strcmp("xor", wordzero) == 0
		|| strcmp("cmp", wordzero) == 0){
			// WORD_OPCODE WORD_REGISTER WORD_REGISTER
			if(sscanf(sourceCode[i], "%s %s %s", &s1, &s2, &s3) != 3){
				printf("[Syntax] GetWords: Incorrect %s usage in line: \n\t%s\n", wordzero, sourceCode[i]);
				exit(1);
			}
			sourceLines[i].wordCount = 3;
			sourceLines[i].words = (t_word *) malloc(sizeof(t_word) * sourceLines[i].wordCount);

			sourceLines[i].words[0].type = WORD_OPCODE;
			strcpy(sourceLines[i].words[0].str, s1);
			sourceLines[i].words[0].value = parseInstructionCode(s1);

			sourceLines[i].words[1].type = WORD_REGISTER;
			strcpy(sourceLines[i].words[1].str, s2);
			sourceLines[i].words[1].value = parseRegisterCode(s2);

			sourceLines[i].words[2].type = WORD_REGISTER;
			strcpy(sourceLines[i].words[2].str, s3);
			sourceLines[i].words[2].value = parseRegisterCode(s3);
		} else if(strcmp("neg", wordzero) == 0
		|| strcmp("shl", wordzero) == 0
		|| strcmp("shr", wordzero) == 0){
			// WORD_OPCODE WORD_REGISTER
			if(sscanf(sourceCode[i], "%s %s", &s1, &s2) != 2){
				printf("[Syntax] GetWords: Incorrect %s usage in line: \n\t%s\n", wordzero, sourceCode[i]);
				exit(1);
			}
			sourceLines[i].wordCount = 2;
			sourceLines[i].words = (t_word *) malloc(sizeof(t_word) * sourceLines[i].wordCount);

			sourceLines[i].words[0].type = WORD_OPCODE;
			strcpy(sourceLines[i].words[0].str, s1);
			sourceLines[i].words[0].value = parseInstructionCode(s1);

			sourceLines[i].words[1].type = WORD_REGISTER;
			strcpy(sourceLines[i].words[1].str, s2);
			sourceLines[i].words[1].value = parseRegisterCode(s2);
		} else if(strcmp("jmp", wordzero) == 0
		|| strcmp("je", wordzero) == 0
		|| strcmp("jne", wordzero) == 0
		|| strcmp("jg", wordzero) == 0
		|| strcmp("jl", wordzero) == 0){
			// WORD_OPCODE WORD_POINTER
			if(sscanf(sourceCode[i], "%s %s", &s1, &i1) != 2){
				f1 = 1; // Pointer is a variable
				if(sscanf(sourceCode[i], "%s %s %s", &s1, &s2) != 2 || !(isalpha(s2[0]) || s2[0] == '.')){
					printf("[Syntax] GetWords: Incorrect %s usage in line: \n\t%s\n", wordzero, sourceCode[i]);
					exit(1);
				}
			}
			sourceLines[i].wordCount = 2;
			sourceLines[i].words = (t_word *) malloc(sizeof(t_word) * sourceLines[i].wordCount);

			sourceLines[i].words[0].type = WORD_OPCODE;
			strcpy(sourceLines[i].words[0].str, s1);
			sourceLines[i].words[0].value = parseInstructionCode(s1);

			sourceLines[i].words[1].type = WORD_POINTER;
			if(f1 == 0){
				sourceLines[i].words[1].value = i1;
				sourceLines[i].words[1].str[0] = '\0';
				if(i1 > 0xFFFF) printf("[Warning] GetWords: Pointer seems to be out of range: 0x%X (PCPUv3 supports 16-bit addressing) in line: \n\t%s\n", i1, sourceCode[i]);
			} else{
				strcpy(sourceLines[i].words[1].str, s2);
			}
		} else if(strcmp("out", wordzero) == 0){
			// WORD_OPCODE WORD_VALUE WORD_REGISTER
			if(sscanf(sourceCode[i], "%s 0x%x %s", &s1, &i1, &s2) != 3){
				printf("[Syntax] GetWords: Incorrect %s usage in line: \n\t%s\n", wordzero, sourceCode[i]);
				exit(1);
			}
			sourceLines[i].wordCount = 3;
			sourceLines[i].words = (t_word *) malloc(sizeof(t_word) * sourceLines[i].wordCount);

			sourceLines[i].words[0].type = WORD_OPCODE;
			strcpy(sourceLines[i].words[0].str, s1);
			sourceLines[i].words[0].value = parseInstructionCode(s1);

			sourceLines[i].words[1].type = WORD_VALUE;
			sourceLines[i].words[1].value = i1;
			sourceLines[i].words[1].str[0] = '\0';
			if(i1 > 0xFF) printf("[Warning] GetWords: Output port seems to be out of range: 0x%X (PCPUv3 supports 8-bit I/O addressing) in line: \n\t%s\n", i1, sourceCode[i]);

			sourceLines[i].words[2].type = WORD_REGISTER;
			strcpy(sourceLines[i].words[2].str, s2);
			sourceLines[i].words[2].value = parseRegisterCode(s2);
		} else if(strcmp("in", wordzero) == 0){
			// WORD_OPCODE WORD_REGISTER WORD_VALUE
			if(sscanf(sourceCode[i], "%s %s 0x%x", &s1, &s2, &i1) != 3){
				printf("[Syntax] GetWords: Incorrect %s usage in line: \n\t%s\n", wordzero, sourceCode[i]);
				exit(1);
			}
			sourceLines[i].wordCount = 3;
			sourceLines[i].words = (t_word *) malloc(sizeof(t_word) * sourceLines[i].wordCount);

			sourceLines[i].words[0].type = WORD_OPCODE;
			strcpy(sourceLines[i].words[0].str, s1);
			sourceLines[i].words[0].value = parseInstructionCode(s1);

			sourceLines[i].words[1].type = WORD_REGISTER;
			strcpy(sourceLines[i].words[1].str, s2);
			sourceLines[i].words[1].value = parseRegisterCode(s2);

			sourceLines[i].words[2].type = WORD_VALUE;
			sourceLines[i].words[2].value = i1;
			sourceLines[i].words[2].str[0] = '\0';
			if(i1 > 0xFF) printf("[Warning] GetWords: Input port seems to be out of range: 0x%X (PCPUv3 supports 8-bit I/O addressing) in line: \n\t%s\n", i1, sourceCode[i]);
		} else if(strcmp("int", wordzero) == 0){
			// WORD_OPCODE
			sourceLines[i].wordCount = 1;
			sourceLines[i].words = (t_word *) malloc(sizeof(t_word) * sourceLines[i].wordCount);

			sourceLines[i].words[0].type = WORD_OPCODE;
			strcpy(sourceLines[i].words[0].str, wordzero);
			sourceLines[i].words[0].value = parseInstructionCode(wordzero);
		} else if(strcmp("db", wordzero) == 0
		|| strcmp("dw", wordzero) == 0
		|| strcmp("dl", wordzero) == 0){
			// WORD_SPECIAL WORD_VARNAME WORD_VALUE|WORD_VALUE2|WORD_VALUE4
			if(sscanf(sourceCode[i], "%s %s 0x%x", &s1, &s2, &i1) != 3){
				f1 = 1;
				if(sscanf(sourceCode[i], "%s %s", &s1, &s2) != 2){
					printf("[Syntax] GetWords: Incorrect %s usage in line: \n\t%s\n", wordzero, sourceCode[i]);
					exit(1);
				}
			}
			sourceLines[i].wordCount = 3;
			sourceLines[i].words = (t_word *) malloc(sizeof(t_word) * sourceLines[i].wordCount);

			sourceLines[i].words[0].type = WORD_SPECIAL;
			strcpy(sourceLines[i].words[0].str, s1);
			sourceLines[i].words[0].value = parseSymbolCode(s1);

			sourceLines[i].words[1].type = WORD_VARNAME;
			strcpy(sourceLines[i].words[1].str, s2);

			switch(sourceLines[i].words[0].value){
				default: break;
				case SYMBOL_DB:
					sourceLines[i].words[2].type = WORD_VALUE;
					break;
				case SYMBOL_DW:
					sourceLines[i].words[2].type = WORD_VALUE2;
					break;
				case SYMBOL_DL:
					sourceLines[i].words[2].type = WORD_VALUE4;
					break;
			}
			if(f1 == 1){ i1 = 0; }
			sourceLines[i].words[2].value = i1;

			stackItems++;
		} else{
			printf("[Syntax] GetWords: Unsupported symbol: %s in line: \n\t%s\n", wordzero, sourceCode[i]);
			exit(1);
		}
	}
}

size_t trimwhitespace(char *out, size_t len, const char *str){
	if(len == 0)
		return 0;
	const char *end;
	size_t out_size;

	while(isspace(*str)) str++;
	if(*str == 0){
		*out = 0;
		return 0;
	}

	end = str + strlen(str) - 1;
	while(end > str && isspace(*end)) end--;
	end++;

	out_size = (end - str) < len-1 ? (end - str) : len-1;

	memcpy(out, str, out_size);
	out[out_size] = 0;

	return out_size;
}

void filePrepare(FILE *fp, char *filename){
	char buf[1024], buf2[1024];
	long fileSize = 0;
	long fileLines = 0;
	int slen;

	fseek(fp, 0L, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	sourceCode = (char **) malloc(sizeof(char *));

	while(fgets(buf, sizeof(buf), fp) != NULL){
		fileLines++;
		slen = trimwhitespace(buf2, strlen(buf), buf);
		if(buf2[0] != ';' && slen > 0){
			codeLines++;
			sourceCode = (char **) realloc((void *) sourceCode, sizeof(char *) * codeLines);
			sourceCode[codeLines - 1] = (char *) malloc(sizeof(char) * (strlen(buf2) + 1));
			strcat(sourceCode[codeLines - 1], buf2);
		}
	}

	printf("FilePrepare: Source file: %s [%d bytes | %d lines]\n", filename, fileSize, fileLines);
	printf("FilePrepare: Source without comments / empty lines: %d lines\n", codeLines);
}

void dumpCodeStack(){
	for(int i = 0; i < codeLines; i++){
		if(sourceLines[i].words[0].type == WORD_SPECIAL) continue;

		printf("%d\t ", i);
		for(int j = 0; j < sourceLines[i].wordCount; j++){
			t_word w = sourceLines[i].words[j];
			switch(w.type){
				default:
					printf("%s ", w.str);
					break;
				case WORD_POINTER:
					if(strlen(w.str) > 0){
						printf("%s ", w.str);
					} else{
						printf("0x%04X ", w.value);
					}
					break;
			}
		}
		printf(" \t|\t ");
		for(int j = 0; j < sourceLines[i].wordCount; j++){
			t_word w = sourceLines[i].words[j];
			switch(w.type){
				default:
					printf("%d ", w.value);
					break;
				case WORD_POINTER:
					if(strlen(w.str) > 0){
						printf("$ ");
					} else{
						printf("0x%X ", w.value);
					}
					break;
			}
		}
		printf("\n");
	}
}

void dumpStack(){
	long offset = 0;
	for(int i = 0; i < stackItems; i++){
		if(stack[i].length == 0) return;
		switch(stack[i].length){
			default:
				printf("0x%04X: %08X as %s\n", offset, stack[i].value, stack[i].name);
				break;
			case 1:
				printf("0x%04X: %02X as %s\n", offset, stack[i].value, stack[i].name);
				break;
			case 2:
				printf("0x%04X: %04X as %s\n", offset, stack[i].value, stack[i].name);
				break;
		}
		offset += stack[i].length;
	}
}

int varExists(char *s){
	for(int i = 0; i < stackItems; i++){
		if(stack[i].length == 0) return 0;

		if(strcmp(s, stack[i].name) == 0) return 1;
	}
	return 0;
}

int getStackPointer(int offset, char *name){
	for(int i = 0; i < stackItems; i++){
		if(stack[i].length == 0) break;

		if(strcmp(stack[i].name, name) == 0){
			return offset;
		}
		offset += stack[i].length;
	}
	return -1;
}

void createStack(){
	stack = (t_datafield *) malloc(sizeof(t_datafield) * stackItems);
	for(int i = 0; i < stackItems; i++){
		stack[i].length = 0;
	}

	long offset = 0;
	for(int i = 0; i < codeLines; i++){
		t_word wordzero = sourceLines[i].words[0];
		if(wordzero.type == WORD_SPECIAL){
			if(wordzero.value == SYMBOL_DB){
				t_word wordname = sourceLines[i].words[1];
				if(varExists(wordname.str)){
					printf("[Scope] CreateStack: Tried to define second variable with the same label: %s\n", wordname.str);
					exit(1);
				}

				t_word wordvalue = sourceLines[i].words[2];
				stack[offset].length = 1;
				stack[offset].value = wordvalue.value;
				strcpy(stack[offset].name, wordname.str);
				offset++;
			} else if(wordzero.value == SYMBOL_DW){
				t_word wordname = sourceLines[i].words[1];
				if(varExists(wordname.str)){
					printf("[Scope] CreateStack: Tried to define second variable with the same label: %s\n", wordname.str);
					exit(1);
				}

				t_word wordvalue = sourceLines[i].words[2];
				stack[offset].length = 2;
				stack[offset].value = wordvalue.value;
				strcpy(stack[offset].name, wordname.str);
				offset++;
			} else if(wordzero.value == SYMBOL_DL){
				t_word wordname = sourceLines[i].words[1];
				if(varExists(wordname.str)){
					printf("[Scope] CreateStack: Tried to define second variable with the same label: %s\n", wordname.str);
					exit(1);
				}

				t_word wordvalue = sourceLines[i].words[2];
				stack[offset].length = 4;
				stack[offset].value = wordvalue.value;
				strcpy(stack[offset].name, wordname.str);
				offset++;
			}
		}
	}
}

void estimateSizes(){
	executive_size = 0;
	stack_size = 0;
	for(int i = 0; i < codeLines; i++){
		t_word wordzero = sourceLines[i].words[0];
		switch(wordzero.type){
			default:
				for(int j = 0; j < sourceLines[i].wordCount; j++){
					t_word w = sourceLines[i].words[j];
					switch(w.type){
						default: break;
						case WORD_OPCODE: executive_size += 1; break;
						case WORD_POINTER: executive_size += 2; break;
						case WORD_REGISTER: executive_size += 1; break;
						case WORD_VALUE: executive_size += 1; break;
					}
				}
				break;
			case WORD_SPECIAL:
				switch(wordzero.value){
					default: break;
					case SYMBOL_DB:
						stack_size += 1;
						break;
					case SYMBOL_DW:
						stack_size += 2;
						break;
					case SYMBOL_DL:
						stack_size += 4;
						break;
				}
				break;
		}
	}
}

void resolveVariables(){
	for(int i = 0; i < codeLines; i++){
		for(int j = 0; j < sourceLines[i].wordCount; j++){
			t_word w = sourceLines[i].words[j];
			switch(w.type){
				default: break;
				case WORD_POINTER:
					if(strlen(w.str) > 0){
						int ptr = getStackPointer(executive_size + 1, w.str);
						if(ptr < 0){
							printf("[Scope] ResolveVariables: Variable: %s has not been declared\n", w.str);
							exit(1);
						} else{
							sourceLines[i].words[j].str[0] = '\0';
							sourceLines[i].words[j].value = ptr;
						}
					}
					break;
			}
		}
	}
}

void assemble(){
	long binsize = executive_size + 1 + stack_size;
	uint8 *binary = (uint8 *) malloc(sizeof(uint8) * binsize);
	for(int i = 0; i < binsize; i++) binary[i] = 0;

	long offset = 0;
	for(int i = 0; i < codeLines; i++){
		t_word wordzero = sourceLines[i].words[0];
		if(wordzero.type != WORD_OPCODE) continue;
		for(int j = 0; j < sourceLines[i].wordCount; j++){
			t_word w = sourceLines[i].words[j];
			switch(w.type){
				default: break;
				case WORD_OPCODE:
					binary[offset] = w.value;
					offset += 1;
					break;
				case WORD_POINTER:
					binary[offset] = (w.value & 0xFF00) >> 8;
					binary[offset + 1] = (w.value & 0x00FF);
					offset += 2;
					break;
				case WORD_REGISTER:
					binary[offset] = w.value;
					offset += 1;
					break;
				case WORD_VALUE:
					binary[offset] = w.value;
					offset += 1;
					break;
			}
		}
	}

	binary[offset] = NUL;
	offset++;

	for(int i = 0; i < stackItems; i++){
		if(stack[i].length == 0) break;

		printf("SI %d.: %X\n", i, stack[i].value);

		switch(stack[i].length){
			default: break;
			case 1:
				binary[offset] = stack[i].value;
				offset++;
				break;
			case 2:
				binary[offset] = (stack[i].value & 0xFF00) >> 8;
				offset++;
				binary[offset] = stack[i].value & 0x00FF;
				offset++;
				break;
			case 4:
				binary[offset] = (stack[i].value & 0xFF000000) >> 24;
				offset++;
				binary[offset] = (stack[i].value & 0x00FF0000) >> 16;
				offset++;
				binary[offset] = (stack[i].value & 0x0000FF00) >> 8;
				offset++;
				binary[offset] = stack[i].value & 0x000000FF;
				offset++;
				break;
		}
	}
	output = binary;
}

int main(int argc, char *argv[]){
	if(argc < 3){
		printf("PCPU Assembler (v3-compatible)\n");
		printf("Usage: %s source.asm output.bin\n", argv[0]);
		return -1;
	}

	FILE *source = fopen(argv[1], "r");
	if(!source){
		printf("Cannot open file: %s\n", argv[1]);
		return -1;
	}

	filePrepare(source, argv[1]);
	getWords();

	dumpCodeStack();
	estimateSizes();
	printf("Estimated executive size: %d bytes\n", executive_size);

	createStack();
	dumpStack();

	resolveVariables();
	dumpCodeStack();

	assemble();

	FILE *binary = fopen(argv[2], "wb");
	if(!binary){
		printf("Cannot open file: %s\n", argv[2]);
		return -1;
	}

	long bytes = fwrite(output, sizeof(output[0]), executive_size + 1 + stack_size, binary);
	printf("Output binary: %s [%d bytes]\n", argv[2], bytes);

	fclose(source);
	fclose(binary);

	return 0;
}
