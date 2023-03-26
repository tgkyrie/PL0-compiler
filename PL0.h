#pragma once
#include <stdio.h>
#include<string>
#include "ArrayInf.h"

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
extern const int gReservedWordsNum;
extern const int gMaxIdTableLen;
extern const int gMaxNumLen;
extern const int gNSym;
extern const int gMaxIdLen;
extern const int gMaxAddress;
extern const int gMaxLevel;
extern const int gMaxInstNum;
extern const int gMaxSym;
extern const int gStackSize;
extern const int gJmpBufSize;
extern int gErr;
extern FILE* infile;

extern int jmp_buf[];




enum symtype
{
	SYM_NULL,
	SYM_IDENTIFIER,
	SYM_NUMBER,
	SYM_PLUS,
	SYM_MINUS,
	SYM_TIMES,
	SYM_SLASH,
	SYM_ODD,
	SYM_EQU,
	SYM_NEQ,
	SYM_LES,
	SYM_LEQ,
	SYM_GTR,
	SYM_GEQ,
	SYM_LPAREN,
	SYM_RPAREN,
	SYM_COMMA,
	SYM_SEMICOLON,
	SYM_LBRACKET,
	SYM_RBRACKET,
	SYM_PERIOD,
	SYM_BECOMES,
    SYM_BEGIN,
	SYM_END,
	SYM_IF,
	SYM_THEN,
	SYM_WHILE,
	SYM_DO,
	SYM_CALL,
	SYM_CONST,
	SYM_VAR,
	SYM_PROCEDURE,
	SYM_PRINT,
	SYM_FOR,
	SYM_COLON,
	SYM_ELSE,
	SYM_SETJMP,
	SYM_LONGJMP
};

enum idtype
{
	ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE,ID_ARRAY
};

enum opcode
{
	LIT, OPR, LOD, STO, CAL, INT, JMP, JPC,STOA,LODA,PRT,LEA,SJP,LJP
};

enum oprcode
{
	OPR_RET, OPR_NEG, OPR_ADD, OPR_MIN,
	OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR,
	OPR_GEQ
};


// typedef struct
// {
// 	int f; // function code
// 	int l; // level
// 	int a; // displacement address
// } instruction;

typedef struct
{
	int funcCode; // function code
	int level; // level
	int address; // displacement address
} instruction;

//////////////////////////////////////////////////////////////////////
// char* err_msg[] =
// {
// /*  0 */    "",
// /*  1 */    "Found ':=' when expecting '='.",
// /*  2 */    "There must be a number to follow '='.",
// /*  3 */    "There must be an '=' to follow the identifier.",
// /*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
// /*  5 */    "Missing ',' or ';'.",
// /*  6 */    "Incorrect procedure name.",
// /*  7 */    "Statement expected.",
// /*  8 */    "Follow the statement is an incorrect symbol.",
// /*  9 */    "'.' expected.",
// /* 10 */    "';' expected.",
// /* 11 */    "Undeclared identifier.",
// /* 12 */    "Illegal assignment.",
// /* 13 */    "':=' expected.",
// /* 14 */    "There must be an identifier to follow the 'call'.",
// /* 15 */    "A constant or variable can not be called.",
// /* 16 */    "'then' expected.",
// /* 17 */    "';' or 'end' expected.",
// /* 18 */    "'do' expected.",
// /* 19 */    "Incorrect symbol.",
// /* 20 */    "Relative operators expected.",
// /* 21 */    "Procedure identifier can not be in an expression.",
// /* 22 */    "Missing ')'.",
// /* 23 */    "The symbol can not be followed by a factor.",
// /* 24 */    "The symbol can not be as the beginning of an expression.",
// /* 25 */    "The number is too great.",
// /* 26 */    "Missing :",
// /* 27 */    "Missing (",
// /* 28 */    "Missing var",
// /* 29 */    "step should not be zero",
// /* 30 */    "Const or Number expected",
// /* 31 */    "',' expected",
// /* 32 */    "There are too many levels."
// };

//////////////////////////////////////////////////////////////////////
// char ch;         // last character read
// int  sym;        // last symbol read
// char id[gMaxIdLen + 1]; // last identifier read
// int  num;        // last number read
// int  cc;         // character count
// int  ll;         // line length
// int  kk;
// int  err;
// int  cx;         // index of current instruction to be generated.
// int  level = 0;
// int  tx = 0;

// char chLastRead;         // last character read
// int  symLastRead;        // last symbol read
// char idLastRead[gMaxIdLen + 1]; // last identifier read
// std::string idLastRead;
// int  numLastRead;        // last number read
// int  chCount;         // character count
// int  lineLen;         // line length
// // int  kk;             //what?
// int  err;
// int  codeIndex;         // index of current instruction to be generated.
// int  level = 0;
// int  tableIndex = 0;

// char line[80];

// instruction code[gMaxInstNum];

extern std::string reservedWords[];

extern int reservedWordsSym[];

extern int reservedCharSym[];
extern char reservedChar[];

// #define MAXINS   14
extern const int gInstTableSize;
extern std::string mnemonic[];

// typedef struct
// {
// 	char name[gMaxIdLen + 1];
// 	int  kind;
// 	int  value;
// 	arrayInf array;
// } comtab;

// comtab table[gMaxIdTableLen];




typedef struct
{
	std::string name;
	int   kind;
	short level;
	short address;
	ArrayInf array;
} mask;


// EOF PL0.h