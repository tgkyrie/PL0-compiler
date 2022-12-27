// pl0 compiler source code

#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "PL0.h"
#include "set.c"
#include "arrayinf.h"

int limit(arrayInf array,int ndim){
	int dim=array.dim;
	int ret=1;
	for(int i=ndim+1;i<dim;i++){
		ret*=array.array[i];
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

//////////////////////////////////////////////////////////////////////
void getch(void)
{
	if (cc == ll)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		while ( (!feof(infile)) // added & modified by alex 01-02-09
			    && ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);
			line[++ll] = ch;
		} // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];
} // getch

//////////////////////////////////////////////////////////////////////
// gets a symbol from input stream.
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];

	while (ch == ' '||ch == '\t')
		getch();

	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		}
		while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy(id, a);
		word[0] = id;
		i = NRW;
		while (strcmp(id, word[i--]));
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else
			sym = SYM_IDENTIFIER;   // symbol is an identifier
	}
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';
			k++;
			getch();
		}
		while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25);     // The number is too great.
	}
	else if (ch == ':')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_BECOMES; // :=
			getch();
		}
		else
		{
			sym = SYM_COLON;       // illegal?
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ;     // >=
			getch();
		}
		else
		{
			sym = SYM_GTR;     // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ;     // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ;     // <>
			getch();
		}
		else
		{
			sym = SYM_LES;     // <
		}
	}
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i)
		{
			sym = ssym[i];
			getch();
		}
		else
		{
			printf("%d\n",(int)(ch));
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction.
void gen(int x, int y, int z)
{
	if (cx > CXMAX)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
} // gen

//////////////////////////////////////////////////////////////////////
// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n)
{
	symset s;
	if (! inset(sym, s1))
	{
		// printf("%d\n",sym);
		error(n);
		s = uniteset(s1, s2);
		while(! inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

//////////////////////////////////////////////////////////////////////
int dx;  // data allocation index

// enter object(constant, variable or procedre) into table.
void enter(int kind,arrayInf* array,char* oldId)
{
	mask* mk;

	tx++;
	strcpy(table[tx].name, oldId);
	// printf("%d\n",kind);
	table[tx].kind = kind;
	switch (kind)
	{
	case ID_CONSTANT:
		if (num > MAXADDRESS)
		{
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num;
		break;
	case ID_VARIABLE:
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE:
		mk = (mask*) &table[tx];
		mk->level = level;
		break;
	case ID_ARRAY:
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address=dx;
		mk->array=*array;
		dx+=limit(*array,-1);
	} // switch
} // enter

//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
int position(char* id)
{
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0);
	return i;
} // position

//////////////////////////////////////////////////////////////////////
void constdeclaration()
{
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES)
		{
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT,NULL,id);
				getsym();
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	} else	error(4);
	 // There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration

void brackets(arrayInf* array){
	if(sym==SYM_LBRACKET){
		getsym();
		if(sym==SYM_IDENTIFIER){
			int i=position(id);
			if(!i){
				//error Undeclared id
			}
			if(table[i].kind!=ID_CONSTANT){
				//error should be const
			}
			push_back(array,table[i].value);
			getsym();
			if(sym!=SYM_RBRACKET){
				//error expect ]
			}
			getsym();
			brackets(array);
		}
		else if(sym==SYM_NUMBER){
			push_back(array,num);
			getsym();
			if(sym!=SYM_RBRACKET){
				//error expect ]
			}
			getsym();
			brackets(array);
		}
		else{
			//error a[k] k should be const or number
		}
	}
	else{
		return;
	}
}

//////////////////////////////////////////////////////////////////////
void vardeclaration(void)
{
	// if (sym == SYM_IDENTIFIER)
	// {
	// 	enter(ID_VARIABLE);
	// 	getsym();
	// }
	if(sym==SYM_IDENTIFIER){
		char oldId[MAXIDLEN+1];
		strcpy(oldId,id);
		getsym();
		arrayInf array;
		arrayInfInit(&array);
		brackets(&array);
		if(array.dim==0){
			//is not array
			enter(ID_VARIABLE,NULL,oldId);
		}
		else{
			enter(ID_ARRAY,&array,oldId);
		}
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

//////////////////////////////////////////////////////////////////////
void listcode(int from, int to)
{
	int i;
	
	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

//////////////////////////////////////////////////////////////////////
bool factor(symset fsys,bool inferNotLvalue)
{
	bool assign_expression(symset fsys);
	void V(symset fsys);
	void setjmp(symset);
	int i;
	symset set;
	bool lvalue=0;
	
	
	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	if (inset(sym, facbegsys))
	{
		if (sym == SYM_IDENTIFIER)
		{
			if ((i = position(id)) == 0)
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				switch (table[i].kind)
				{
					mask* mk;
				case ID_CONSTANT:
					gen(LIT, 0, table[i].value);
					break;
				case ID_VARIABLE:
					lvalue=1;
					mk = (mask*) &table[i];
					if(inferNotLvalue)gen(LOD, level - mk->level, mk->address);
					else gen(LEA,level-mk->level,mk->address);
					break;
				case ID_PROCEDURE:
					error(21); // Procedure identifier can not be in an expression.
					break;
				case ID_ARRAY:
					lvalue=1;
					V(fsys);
					if(inferNotLvalue){
						mk=(mask*)&table[i];
						gen(LODA,level-mk->level,0);
					}
					break;
				} // switch
			}
			if(table[i].kind!=ID_ARRAY)getsym();
		}
		else if (sym == SYM_NUMBER)
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);
			getsym();
			// printf("%d\n",sym);
		}
		else if (sym==SYM_SETJMP){
			setjmp(fsys);
		}
		else if (sym == SYM_LPAREN)
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			// expression(set);
			lvalue=assign_expression(set);
			destroyset(set);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		// else if(sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		// {  
		// 	 getsym();
		// 	 factor(fsys);
		// 	 gen(OPR, 0, OPR_NEG);
		// }
		symset set1=uniteset(fsys,createset(SYM_RBRACKET,SYM_RPAREN,SYM_COMMA,SYM_BECOMES,SYM_NULL));
		test(set1, createset(SYM_LPAREN, SYM_NULL), 23);
	} // if
	return lvalue&&(inferNotLvalue==0);
} // factor

//////////////////////////////////////////////////////////////////////
bool term(symset fsys,bool inferNotLvalue)
{
	int mulop;	symset set;
	bool lvalue=0;
	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_NULL));

	if(sym==SYM_MINUS){
		term(fsys,1);
		gen(OPR,0,OPR_NEG);
	}
	else{
		lvalue=factor(set,inferNotLvalue);
		if(lvalue&&(sym==SYM_TIMES||sym==SYM_SLASH)){
			gen(LODA,0,0);
			lvalue=0;
		}
		while (sym == SYM_TIMES || sym == SYM_SLASH)
		{
			mulop = sym;
			getsym();
			factor(set,1);
			if (mulop == SYM_TIMES)
			{
				gen(OPR, 0, OPR_MUL);
			}
			else
			{
				gen(OPR, 0, OPR_DIV);
			}
		} // while
	}
	destroyset(set);
	return lvalue&&(!inferNotLvalue);
} // term

//////////////////////////////////////////////////////////////////////
bool expression(symset fsys,bool inferNotLvalue)
{
	int addop;
	symset set;
	bool lvalue=0;
	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));

	lvalue=term(set,inferNotLvalue);
	if(lvalue&&(sym == SYM_PLUS || sym == SYM_MINUS)){
		gen(LODA,0,0);
		lvalue=0;
	}
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
		getsym();
		term(set,1);
		if (addop == SYM_PLUS)
		{
			gen(OPR, 0, OPR_ADD);
		}
		else
		{
			gen(OPR, 0, OPR_MIN);
		}
	} // while
	destroyset(set);
	return lvalue&&(!inferNotLvalue);
} // expression


bool condition_expression(symset fsys)
{
	int relop;
	symset set;
	bool lvalue=0;
	set = uniteset(fsys, relset);

	lvalue=expression(set,0);
	// lvalue=term(set,0);
	if(lvalue&&inset(sym,relset)){
		gen(LODA,0,0);
		lvalue=0;
	}
	while (inset(sym,relset))
	{
		relop = sym;
		getsym();
		// term(set,1);
		expression(fsys,1);
		switch (relop)
			{
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
	} // while
	destroyset(set);
	return lvalue;
} // expression

void assign_expression_(symset fsys){
	if(sym==SYM_BECOMES){
		getsym();
		bool lvalue=condition_expression(fsys);
		//语义分析，若接下来对上面这个exp赋值，检查exp是否是左值
		bool null=0;
		if(sym==SYM_BECOMES){
			if(!lvalue){
				//error this exp is not a lvalue
			}
		}
		else null=1;
		assign_expression_(fsys);
		if(null){
			if(lvalue)gen(LODA,0,0);
		}
		else{
			gen(STOA,0,1);
		}
		//if (exp1 is lvalue) gen(LODA,0,0);

	}
	else {
		//is null;
	}
}

bool assign_expression(symset fsys){
	bool ret_lvalue=0;
	bool lvalue=condition_expression(fsys);
	//语义分析，若接下来对上面这个exp赋值，检查exp是否是左值
	int null=0;
	if(sym==SYM_BECOMES){
		if(!lvalue){
			//error this exp is not a lvalue
		}
	}
	else {
		null=1;
	}
	assign_expression_(fsys);
	if(null){
		if(lvalue){
			gen(LODA,0,0);
			ret_lvalue=1;
		}
	}
	else{
		gen(STOA,0,1);
	}
	return ret_lvalue;
	// if(sym==SYM_SEMICOLON){ 
	// 	//一个单独的赋值或条件表达式自成一行，
	// 	//并没有任何作用，后续不会被弹出，所以此处需要弹栈
	// 	// stack pop;
	// }
	// else {
	// 	// if(exp1 is lvalue){
	// 	// 		gen(LODA,0,0);
	// 	// } 
	// }
}

void setjmp(symset fsys)
{
	// void expression(symset);
	getsym();
	if(sym == SYM_LPAREN)
	{
		getsym();
		// expression(fsys);
		assign_expression(fsys);
		if(sym == SYM_RPAREN)
		{
			getsym();
			gen(SJP,0,0);
		}
		else{
			// error: missing ')'
		}
	}
	else{
		// error: expecting '('
	}
}


void longjmp(symset fsys)
{
	// void expression(symset);
	getsym();
	if(sym == SYM_LPAREN)
	{
		getsym();
		// expression(fsys);
		assign_expression(fsys);
		getsym();
		// expression(fsys);
		assign_expression(fsys);
		if(sym == SYM_RPAREN)
		{
			getsym();
			gen(LJP,0,0);
		}
		else{
			// error: missing ')'
		}
	}
	else{
		// error: missing '('
	}
}

struct Node{
	int used;
	int in0;
	int in1;
};

struct Node nodeList[CXMAX];
int optimizeStack[CXMAX];
instruction codeBuf[CXMAX];
void optimize(int begin,int end){

	// int* parent =(int*)malloc(sizeof(int)*(end-begin));
	int top=-1;
	for(int idx=begin;idx<end;idx++){
		instruction i=code[idx];
		nodeList[idx].used=0;
		nodeList[idx].in0=nodeList[idx].in1=-1;
		switch (i.f)
		{
		case LIT:
		case SJP:
		case LOD:
		case LEA:
			optimizeStack[++top]=idx;		
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_NEG:
				nodeList[idx].in0=optimizeStack[top];
				optimizeStack[top]=idx;
				break;
			case OPR_ADD:
			case OPR_MIN:
			case OPR_MUL:
			case OPR_DIV:
			case OPR_EQU:
			case OPR_NEQ:
			case OPR_LES:
			case OPR_GEQ:
			case OPR_GTR:
			case OPR_LEQ:
				nodeList[idx].in0=optimizeStack[top];
				nodeList[idx].in1=optimizeStack[top-1];
				optimizeStack[--top]=idx;
				break;
			} // switch
			break;
		case LODA:
			nodeList[idx].in0=optimizeStack[top];
			optimizeStack[top]=idx;
			break;
		case STO:
			nodeList[idx].in0=optimizeStack[top];
			top--;
			break;
		case STOA:
			if(i.a==0){
				nodeList[idx].in0=optimizeStack[top--];
				nodeList[idx].in1=optimizeStack[top--];
			}
			else{
				nodeList[idx].in0=optimizeStack[top];
				nodeList[idx].in1=optimizeStack[top-1];
				optimizeStack[--top]=idx;
			}
			break;
		default:
			break;
		}
	}
	for(int idx=end-1;idx>=begin;idx--){
		struct Node n=nodeList[idx];
		instruction i=code[idx];
		if(n.used){
			nodeList[idx].used=1;
			if(n.in0!=-1)nodeList[n.in0].used=1;
			if(n.in1!=-1)nodeList[n.in1].used=1;
		}
		else{
			if(i.f==STOA){
				code[idx].a=0;
				nodeList[idx].used=1;
				if(n.in0!=-1)nodeList[n.in0].used=1;
				if(n.in1!=-1)nodeList[n.in1].used=1;
			}
			else if(i.f==STO){
				nodeList[idx].used=1;
				if(n.in0!=-1)nodeList[n.in0].used=1;
				if(n.in1!=-1)nodeList[n.in1].used=1;
			}
			else if(i.f==SJP){
				code[idx].a=1;
				nodeList[idx].used=1;
				if(n.in0!=-1)nodeList[n.in0].used=1;
				if(n.in1!=-1)nodeList[n.in1].used=1;
			}
		}
	}
	int usednum=0;
	for(int idx=begin;idx<end;idx++){
		if(nodeList[idx].used){
			code[begin+usednum]=code[idx];
			usednum++;
		}
	}
	cx=begin+usednum;
	
}


//////////////////////////////////////////////////////////////////////
void condition(symset fsys)
{
	int relop;
	symset set;

	if (sym == SYM_ODD)
	{
		getsym();
		// expression(fsys);
		assign_expression(fsys);
		gen(OPR, 0, 6);
	}
	else
	{
		set = uniteset(relset, fsys);
		// expression(set);
		assign_expression(set);
		destroyset(set);
		if (! inset(sym, relset))
		{
			error(20);
		}
		else
		{
			relop = sym;
			getsym();
			// expression(fsys);
			assign_expression(fsys);
			switch (relop)
			{
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // else
	} // else
} // condition

void paralist_(symset fsys,int* len){
	if(sym==SYM_COMMA){
		getsym();
		// expression(fsys);
		assign_expression(fsys);
		int paralist1_len;
		paralist_(fsys,&paralist1_len);
		*len=paralist1_len+1;
	}
	else{
		*len=0;
	}
}

void paralist(symset fsys,int* len){
	// expression(fsys);
	assign_expression(fsys);
	int paralist_len;
	paralist_(fsys,&paralist_len);
	*len=paralist_len+1;

}


void Elist_(symset fsys,arrayInf array,int ndim){
	if(sym==SYM_COMMA){// ,
		if(ndim>=array.dim){
			// printf("ndim > array.dim\n");
			//error; array have array.dim dims,but get ndim dims
		}
		getsym();
		// expression(fsys);
		assign_expression(fsys);
		gen(LIT,0,limit(array,ndim));
		gen(OPR,0,OPR_MUL);
		Elist_(fsys,array,ndim+1);
		gen(OPR,0,OPR_ADD);
	}
	else if(ndim<array.dim-1){
		// printf("ndim < array.dim-1\n");
		//error; array have array.dim dims,but get ndim dims
	}
	else{
		gen(LIT,0,0);
	}
}

void Elist(symset fsys,arrayInf array){
	// expression(fsys);
	assign_expression(fsys);
	// printf("%d\n",sym);
	//place*limit
	gen(LIT,0,limit(array,0));
	gen(OPR,0,OPR_MUL);
	Elist_(fsys,array,1);
	gen(OPR,0,OPR_ADD);
}

void V_(symset fsys,arrayInf array){
	if(sym==SYM_LBRACKET){
		getsym();
		Elist(fsys,array);
		if(sym!=SYM_RBRACKET){
			//error expect ]
			// printf("expect ]\n");
		}
		getsym();
	}
	else{
		gen(LIT,0,0);
	}
	return;
}

void V(symset fsys){
	if(sym!=SYM_IDENTIFIER){
		//error expect id
	}
	int i=position(id);
	if(!i){
		error(11); //Undeclared identifier.
	}
	else if(!(table[i].kind==ID_VARIABLE||table[i].kind==ID_ARRAY)){
		// printf("%d",table[i].kind);
		error(12); //Illegal assignment.
		i=0;
	}
	mask* mk=(mask*)&table[i];
	getsym();
	if(sym==SYM_LBRACKET&&table[i].kind!=ID_ARRAY){
		//error; id is not array
	}
	else if(sym!=SYM_LBRACKET&&table[i].kind==ID_ARRAY){
		//error; expect [
	}
	V_(fsys,table[i].array);
	gen(LEA,level-mk->level,mk->address);
	gen(OPR,0,OPR_ADD);
}

void low(int i){
	if(sym==SYM_NUMBER){
		getsym();
		gen(LIT,0,num);
	}
	else if(sym==SYM_CONST){
		
	}
	else {
		//error expect const or number
	}
}

void high(){

}

void step(){

}

void rangeList(symset fsys){
	if(sym==SYM_LPAREN){
		getsym();

		// low


		if(sym==SYM_COMMA){
			getsym();
			//high

			if(sym==SYM_COMMA){
				getsym();
				//step
				if(sym==SYM_RPAREN){
					getsym();
				}
				else {
					//error expect )
				}
			}
			else if(sym==SYM_RPAREN){
				//step=1;
				getsym();
			}
			else {
				//error expect ) or ,
			}
		}
		else{
			//error expect ,
		}
	}
	else {
		//error expect (
	}
}
typedef struct {
	int low;
	int high;
	int step;
} rangeRetVal;
rangeRetVal range(symset fsys){
	int low=1,high=1,step=1,var_pos;
	if(sym==SYM_LPAREN){
		getsym();
		if(sym==SYM_NUMBER||sym==SYM_CONST){
			getsym();
			if(sym==SYM_CONST){
				int i=position(id);
				low=table[i].value;
			}
			else {
				low=num;
			}
			if(sym==SYM_COMMA){
				getsym();
				if(sym==SYM_NUMBER||sym==SYM_CONST){
					getsym();
					if(sym==SYM_CONST){
						int i=position(id);
						high=table[i].value;
					}
					else {
						high=num;
					}
					if(sym==SYM_COMMA){
						getsym();
						if(sym==SYM_NUMBER||sym==SYM_CONST){
							//step
							getsym();
							if(sym==SYM_CONST){
								int i=position(id);
								step=table[i].value;
							}
							else {
								step=num;
							}
							if(sym==SYM_RPAREN){
								getsym();
							}
							else {
								//error expect )
								error(22);
							}
						}
						else {
							error(30);
						}
					}
					else if(sym==SYM_RPAREN){
						//step=1;
						getsym();
					}
					else {
						error(22);
					}
				}
				else {
					error(30);
				}
			}
			else {
				error(31);
			}

		}
		else {
			error(30);
		}
	}
	else{
		error(27);
	}
	rangeRetVal ret={low,high,step};
	return ret;
}

void for_statement(symset fsys){
	void statement(symset fsys);
	int cx1,cx2;
	if(sym==SYM_FOR){
		getsym();
		if(sym==SYM_LPAREN){
			getsym();
			if(sym==SYM_VAR){
				getsym();
				if(sym==SYM_IDENTIFIER){
					getsym();
					char oldId[MAXIDLEN+1];
					mask* mk;
					strcpy(oldId,id);
					enter(ID_VARIABLE,NULL,id);
					mk=(mask*)&table[tx];
					if(sym==SYM_COLON){
						getsym();
						rangeRetVal ret=range(fsys);
						if(sym==SYM_RPAREN){
							getsym();
							// i=low
							gen(LIT,0,ret.low);
							gen(STO,level-mk->level,mk->address);
							cx1=cx;
							// if not i<=high or i>=high jmp
							gen(LOD,level-mk->level,mk->address);
							gen(LIT,0,ret.high);
							if(ret.step>0)gen(OPR,0,OPR_LEQ);
							else if(ret.step<0)gen(OPR,0,OPR_GEQ);
							else {
								error(29);
								gen(OPR,0,0);
							}
							cx2=cx;
							gen(JPC,0,0);
							statement(fsys);
							// i=i+step
							gen(LOD,level-mk->level,mk->address);
							gen(LIT,0,ret.step);
							gen(OPR,0,OPR_ADD);
							gen(STO,level-mk->level,mk->address);
							//jmp
							gen(JMP,0,cx1);
							code[cx2].a=cx;
							tx--;
						}
						else{
							error(22);
						}
					}
					else {
						error(26);
					}

				}
				else {
					error(4);
				}

			}
			else{
				error(28);
			}
		}
		else {
			error(27);
		}
	}
	else {
		//error 
		//expect for 
	}
}
int last_cx1;
void statement(symset fsys)
{
	int i,cx1,cx2;
	symset set1, set;

	// if (sym == SYM_IDENTIFIER)
	// { // variable assignment
	// 	mask* mk;
	// 	if (! (i = position(id)))
	// 	{
	// 		error(11); // Undeclared identifier.
	// 	}
	// 	// else if (table[i].kind != ID_VARIABLE)
	// 	// {

	// 	// 	error(12); // Illegal assignment.
	// 	// 	i = 0;
	// 	// }
	// 	V(fsys);
	// 	// getsym();
	// 	if (sym == SYM_BECOMES)
	// 	{
	// 		getsym();
	// 	}
	// 	else
	// 	{
	// 		error(13); // ':=' expected.
	// 	}
	// 	expression(fsys);
	// 	mk = (mask*) &table[i];
	// 	if (i)
	// 	{
	// 		if(table[i].kind==ID_VARIABLE){
	// 			gen(STO, level - mk->level, mk->address);
	// 		}
	// 		else if (table[i].kind==ID_ARRAY){
	// 			gen(STOA,level-mk->level,0);
	// 		}
	// 	}

	// }
	symset assignExpBeginSet=createset(SYM_IDENTIFIER,SYM_NUMBER,SYM_MINUS,SYM_CONST,SYM_LPAREN);
	if (inset(sym,assignExpBeginSet))
	{ // variable assignment
		int begincx=cx;
		assign_expression(fsys);
		optimize(begincx,cx);
	}
	else if (sym == SYM_CALL)
	{ // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if (! (i = position(id)))
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				mask* mk;
				mk = (mask*) &table[i];
				gen(CAL, level - mk->level, mk->address);
			}
			else
			{
				error(15); // A constant or variable can not be called. 
			}
			getsym();
		}
	} 
	else if (sym == SYM_IF)
	{ // if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		// condition(set);
		assign_expression(set);
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}
		cx1 = cx;
		gen(JPC, 0, 0);
		statement(fsys);
		last_cx1=cx1;
		code[cx1].a = cx;
		test(fsys, phi, 19);
		return ;
	}
	else if(sym == SYM_ELSE){
		if(last_cx1==-1){
			//error no matched if
		}
		getsym();
		cx2=cx;
		gen(JMP,0,0);
		code[last_cx1].a=cx;
		statement(fsys);
		code[cx2].a=cx;
	}
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys))
		{
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);
			}
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_WHILE)
	{ // while statement
		cx1 = cx;
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		// condition(set);
		assign_expression(set);
		destroyset(set1);
		destroyset(set);
		cx2 = cx;
		gen(JPC, 0, 0);
		if (sym == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		statement(fsys);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
	}
	else if(sym == SYM_PRINT){
		getsym();
		if(sym==SYM_LPAREN){
			getsym();
			if(sym==SYM_RPAREN){
				getsym();
				gen(PRT,0,0);
			}
			else{
				int len;
				paralist(fsys,&len);
				if(sym==SYM_RPAREN){
					getsym();
					gen(PRT,0,len);
				}
				else {
					//error expect )
				}
			}
			
		}	
		else {
			//error expect (
		}
	}
	else if(sym==SYM_FOR){
		for_statement(fsys);
	}
	else if(sym==SYM_LONGJMP){
		longjmp(fsys);
	}
	test(fsys, phi, 19);
	// last_cx1=-1;
} // statement



//////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	int cx0; // initial code index
	mask* mk;
	int block_dx;
	int savedTx;
	symset set1, set;

	dx = 3;
	block_dx = dx;
	mk = (mask*) &table[tx];
	mk->address = cx;
	gen(JMP, 0, 0);
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do
	{
		if (sym == SYM_CONST)
		{ // constant declarations
			getsym();
			do
			{
				constdeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON) //;
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);
		} // if

		if (sym == SYM_VAR)
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);
		} // if
		block_dx = dx; //save dx before handling procedure call!
		while (sym == SYM_PROCEDURE)
		{ // procedure declarations
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE,NULL,id);
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}


			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}

			level++;
			savedTx = tx;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);
			destroyset(set1);
			destroyset(set);
			tx = savedTx;
			level--;

			if (sym == SYM_SEMICOLON)
			{
				getsym();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6);
				destroyset(set1);
				destroyset(set);
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}
		} // while
		dx = block_dx; //restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	}
	while (inset(sym, declbegsys));

	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	gen(INT, 0, block_dx);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx);
} // block

//////////////////////////////////////////////////////////////////////
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;
	
	while (levelDiff--)
		b = stack[b];
	return b;
} // base

//////////////////////////////////////////////////////////////////////
// interprets and executes codes.
void interpret()
{
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register
	int buf_idx;
	int ljp_ret;
	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];
		switch (i.f)
		{
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				top = b - 1;
				pc = stack[top + 3];
				b = stack[top + 2];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
				break;
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
				break;
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
				break;
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case LODA:
			// stack[top]=stack[base(stack,b,i.l)+stack[top]];
			stack[top]=stack[stack[top]];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			// printf("%d\n", stack[top]);
			top--;
			break;
		case STOA:
			// stack[base(stack,b,i.l)+stack[top-1]]=stack[top];
			stack[stack[top-1]]=stack[top];
			// printf("%d\n", stack[top]);
			if(i.a==0){
				top-=2;
			}
			else {
				stack[top-1]=stack[top];
				top--;
			}
			break;
		case LEA:
			stack[++top]=base(stack,b,i.l)+i.a;
			break;
		case CAL:
			stack[top + 1] = base(stack, b, i.l);
			// generate new block mark
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.a;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;
		case PRT:
			if(i.a==0){
				printf("\n");
			}
			else{
				for(int k=top-i.a+1;k<=top;k++){
					printf("%d ",stack[k]);
				}
				top-=i.a;
			}
			break;
		case SJP:
			buf_idx = (stack[top--] % 4) * 1024;
			jmp_buf[buf_idx + 1] = pc;
			jmp_buf[buf_idx + 2] = b;
			jmp_buf[buf_idx + 3] = top;
			jmp_buf[buf_idx + 4] = i.f;
			jmp_buf[buf_idx + 5] = i.l;
			jmp_buf[buf_idx + 6] = i.a;
			memcpy(jmp_buf + 7, stack + b, top - b + 1);
			if(i.a==0){
				stack[++top] = 0;
			}
			break;
		case LJP:
			ljp_ret = (stack[top--]);
			buf_idx = (stack[top--] % 4) * 1024;
			pc = jmp_buf[buf_idx + 1];
			b = jmp_buf[buf_idx + 2];
			top = jmp_buf[buf_idx + 3];
			i.f = jmp_buf[buf_idx + 4];
			i.l = jmp_buf[buf_idx + 5];
			i.a = jmp_buf[buf_idx + 6];
			memcpy(stack + b, jmp_buf + buf_idx, top - b + 1);
			stack[++top] = ljp_ret;
			break;
		} // switch
	}
	while (pc);

	printf("\nEnd executing PL/0 program.\n");
} // interpret

//////////////////////////////////////////////////////////////////////
void main ()
{
	FILE* hbin;
	char s[80];
	int i;
	symset set, set1, set2;

	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);
	
	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE,SYM_FOR,SYM_ELSE,SYM_LONGJMP,SYM_NULL);
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS,SYM_SETJMP, SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;

	getsym();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	block(set);
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);
} // main

//////////////////////////////////////////////////////////////////////
// eof pl0.c
