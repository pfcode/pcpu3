#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../src/defines.h"

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
#define WORD_VALUE 6		// e.g. 0x20
#define WORD_CONST 7		// e.g. #42
#define WORD_CONSTCHAR 8	// e.g. 'x'
#define WORD_CONSTSTR 9		// e.g. "Hello World!"
#define WORD_LABELPTR 10	// e.g. .main

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

char * strtolower(char *string){
	char *cp;

	cp = string;
	while(*cp) {
		*cp = tolower(*cp);
		cp++;
	}
	return(string);
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
		|| strcmp("get32", wordzero) == 0){
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


	return 0;
}
