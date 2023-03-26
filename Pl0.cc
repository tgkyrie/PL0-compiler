#pragma once
#include <stdio.h>
#include<string>
#include "ArrayInf.h"
#include"PL0.h"

// #define NRW        16     // number of reserved words
// #define TXMAX      500    // length of identifier table
// #define MAXNUMLEN  14     // maximum number of digits in numbers
// #define NSYM       12     // maximum number of symbols in array ssym and csym
// #define MAXIDLEN   10     // length of identifiers

// #define MAXADDRESS 32767  // maximum address
// #define MAXLEVEL   32     // maximum depth of nesting block
// #define CXMAX      500    // size of code array

// #define MAXSYM     30     // maximum number of symbols  

// #define STACKSIZE  1000   // maximum storage

// #define JMPMAX 4096

// typedef unsigned char bool;
const int gReservedWordsNum=16;
const int gMaxIdTableLen=500;
const int gMaxNumLen=14;
const int gNSym=12;
const int gMaxIdLen=10;
const int gMaxAddress=32767;
const int gMaxLevel=32;
const int gMaxInstNum=500;
const int gMaxSym=30;
const int gStackSize=1000;
const int gJmpBufSize=4096;
int gErr=0;
FILE* infile;

int jmp_buf[gJmpBufSize];



std::string reservedWords[gReservedWordsNum + 1] =
{
	"", /* place holder */
	"begin", "call", "const", "do", "end","if",
	"odd", "procedure", "then", "var", "while","print","for","else","setjmp","longjmp"
};

int reservedWordsSym[gReservedWordsNum + 1] =
{
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE,
	SYM_PRINT,SYM_FOR,SYM_ELSE,SYM_SETJMP,SYM_LONGJMP
};

int reservedCharSym[gNSym + 1] =
{
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON,
	SYM_LBRACKET,SYM_RBRACKET
};

char reservedChar[gNSym + 1] =
{
	' ', '+', '-', '*', '/', '(', ')', '=', ',', '.', ';','[',']'
};

// #define MAXINS   14
const int gInstTableSize=14;
std::string mnemonic[gInstTableSize] =
{
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC","STOA","LODA","PRT","LEA","SJP","LJP"
};



// EOF PL0.h