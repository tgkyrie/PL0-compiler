#include"Parser.h"
#include"error.h"
void Parser::test(symset s1, symset s2, int n)
{
	symset s;
	if (! inset(Lex::sym(), s1))
	{
		// printf("%d\n",Lex::sym());
		error(n);
		s = uniteset(s1, s2);
		while(! inset(Lex::sym(), s))
			getsym();
		destroyset(s);
	}
} // test
void Parser::constdeclaration()
{
	if (Lex::sym() == SYM_IDENTIFIER)
	{
		getsym();
		if (Lex::sym() == SYM_EQU || Lex::sym() == SYM_BECOMES)
		{
			if (Lex::sym() == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (Lex::sym() == SYM_NUMBER)
			{
				idTable_.enter(ID_CONSTANT,NULL,Lex::id(),level_);
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

void Parser::brackets(ArrayInf& array){
	if(Lex::sym()==SYM_LBRACKET){
		getsym();
		if(Lex::sym()==SYM_IDENTIFIER){
			int i=idTable_.position(Lex::id());
			if(i==-1){
				//error Undeclared Lex::id()
			}
			if(idTable_.getKind(i)!=ID_CONSTANT){
				//error should be const
			}
			// push_back(array,table[i].value);
            array.push_back(idTable_.getVal(i));
			getsym();
			if(Lex::sym()!=SYM_RBRACKET){
				//error expect ]
			}
			getsym();
			brackets(array);
		}
		else if(Lex::sym()==SYM_NUMBER){
			// push_back(array,Lex::num());
            array.push_back(Lex::num());
			getsym();
			if(Lex::sym()!=SYM_RBRACKET){
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
void Parser::vardeclaration(void)
{
	// if (Lex::sym() == SYM_IDENTIFIER)
	// {
	// 	idTable_.enter(ID_VARIABLE);
	// 	getsym();
	// }
	if(Lex::sym()==SYM_IDENTIFIER){
		// char oldId[gMaxIdLen+1];
		// strcpy(oldId,Lex::id());
        std::string id=Lex::id();
		getsym();
		ArrayInf array;
		// arrayInfInit(&array);
		brackets(array);
		if(array.dim()==0){
			//is not array
			idTable_.enter(ID_VARIABLE,NULL,id,level_);
		}
		else{
			idTable_.enter(ID_ARRAY,&array,id,level_);
		}
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

//////////////////////////////////////////////////////////////////////
// void Parser::listcode(int from, int to)
// {
// 	int i;
	
// 	printf("\n");
// 	for (i = from; i < to; i++)
// 	{
// 		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].funcCode], code[i].level_, code[i].address);
// 	}
// 	printf("\n");
// } // listcode

//////////////////////////////////////////////////////////////////////
bool Parser::factor(symset fsys,bool inferNotLvalue)
{
	// bool assign_expression(symset fsys);
	// void V(symset fsys);
	// void setjmp(symset);
	int i;
	symset set;
	bool lvalue=0;
	
	
	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	if (inset(Lex::sym(), facbegsys))
	{
		if (Lex::sym() == SYM_IDENTIFIER)
		{
			if ((i = idTable_.position(Lex::id())) == -1)
			{
				error(11); // Undeclared identifier.
			}
			else
			{
                short varLevel=idTable_.getLevel(i);
                short varAddr=idTable_.getAddress(i);
				switch (idTable_.getKind(i))
				{
					// mask* mk;
				case ID_CONSTANT:
					code_.gen(LIT, 0, idTable_.getVal(i));
					break;
				case ID_VARIABLE:
					lvalue=1;
					// mk = (mask*) &table[i];
					if(inferNotLvalue)code_.gen(LOD, level_ - varLevel, varAddr);
					else code_.gen(LEA,level_-varLevel,varAddr);
					break;
				case ID_PROCEDURE:
					error(21); // Procedure identifier can not be in an expression.
					break;
				case ID_ARRAY:
					lvalue=1;
					V(fsys);
					if(inferNotLvalue){
						// mk=(mask*)&table[i];
						code_.gen(LODA,level_-varLevel,0);
					}
					break;
				} // switch
			}
			if(idTable_.getKind(i)!=ID_ARRAY)getsym();
		}
		else if (Lex::sym() == SYM_NUMBER)
		{
			if (Lex::num() > gMaxAddress)
			{
				error(25); // The number is too great.
				Lex::setNum(0);
			}
			code_.gen(LIT, 0, Lex::num());
			getsym();
			// printf("%d\n",Lex::sym());
		}
		else if (Lex::sym()==SYM_SETJMP){
			setjmp(fsys);
		}
		else if (Lex::sym() == SYM_LPAREN)
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			// expression(set);
			lvalue=assign_expression(set);
			destroyset(set);
			if (Lex::sym() == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		// else if(Lex::sym() == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		// {  
		// 	 getsym();
		// 	 factor(fsys);
		// 	 code_.gen(OPR, 0, OPR_NEG);
		// }
		symset set1=uniteset(fsys,createset(SYM_RBRACKET,SYM_RPAREN,SYM_COMMA,SYM_BECOMES,SYM_NULL));
		test(set1, createset(SYM_LPAREN, SYM_NULL), 23);
	} // if
	return lvalue&&(inferNotLvalue==0);
} // factor

//////////////////////////////////////////////////////////////////////
bool Parser::term(symset fsys,bool inferNotLvalue)
{
	int mulop;	symset set;
	bool lvalue=0;
	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_NULL));

	if(Lex::sym()==SYM_MINUS){
		getsym();
		term(fsys,1);
		code_.gen(OPR,0,OPR_NEG);
	}
	else{
		lvalue=factor(set,inferNotLvalue);
		if(lvalue&&(Lex::sym()==SYM_TIMES||Lex::sym()==SYM_SLASH)){
			code_.gen(LODA,0,0);
			lvalue=0;
		}
		while (Lex::sym() == SYM_TIMES || Lex::sym() == SYM_SLASH)
		{
			mulop = Lex::sym();
			getsym();
			factor(set,1);
			if (mulop == SYM_TIMES)
			{
				code_.gen(OPR, 0, OPR_MUL);
			}
			else
			{
				code_.gen(OPR, 0, OPR_DIV);
			}
		} // while
	}
	destroyset(set);
	return lvalue&&(!inferNotLvalue);
} // term

//////////////////////////////////////////////////////////////////////
bool Parser::expression(symset fsys,bool inferNotLvalue)
{
	int addop;
	symset set;
	bool lvalue=0;
	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));

	lvalue=term(set,inferNotLvalue);
	if(lvalue&&(Lex::sym() == SYM_PLUS || Lex::sym() == SYM_MINUS)){
		code_.gen(LODA,0,0);
		lvalue=0;
	}
	while (Lex::sym() == SYM_PLUS || Lex::sym() == SYM_MINUS)
	{
		addop = Lex::sym();
		getsym();
		term(set,1);
		if (addop == SYM_PLUS)
		{
			code_.gen(OPR, 0, OPR_ADD);
		}
		else
		{
			code_.gen(OPR, 0, OPR_MIN);
		}
	} // while
	destroyset(set);
	return lvalue&&(!inferNotLvalue);
} // expression


bool Parser::condition_expression(symset fsys)
{
	int relop;
	symset set;
	bool lvalue=0;
	set = uniteset(fsys, relset);

	lvalue=expression(set,0);
	// lvalue=term(set,0);
	if(lvalue&&inset(Lex::sym(),relset)){
		code_.gen(LODA,0,0);
		lvalue=0;
	}
	while (inset(Lex::sym(),relset))
	{
		relop = Lex::sym();
		getsym();
		// term(set,1);
		expression(fsys,1);
		switch (relop)
			{
			case SYM_EQU:
				code_.gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				code_.gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				code_.gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				code_.gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				code_.gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				code_.gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
	} // while
	destroyset(set);
	return lvalue;
} // expression

void Parser::assign_expression_(symset fsys){
	if(Lex::sym()==SYM_BECOMES){
		getsym();
		bool lvalue=condition_expression(fsys);
		//语义分析，若接下来对上面这个exp赋值，检查exp是否是左值
		bool null=0;
		if(Lex::sym()==SYM_BECOMES){
			if(!lvalue){
				//error this exp is not a lvalue
			}
		}
		else null=1;
		assign_expression_(fsys);
		if(null){
			if(lvalue)code_.gen(LODA,0,0);
		}
		else{
			code_.gen(STOA,0,1);
		}
		//if (exp1 is lvalue) code_.gen(LODA,0,0);

	}
	else {
		//is null;
	}
}

bool Parser::assign_expression(symset fsys){
	bool ret_lvalue=0;
	bool lvalue=condition_expression(fsys);
	//语义分析，若接下来对上面这个exp赋值，检查exp是否是左值
	int null=0;
	if(Lex::sym()==SYM_BECOMES){
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
			code_.gen(LODA,0,0);
			ret_lvalue=1;
		}
	}
	else{
		code_.gen(STOA,0,1);
	}
	return ret_lvalue;
	// if(Lex::sym()==SYM_SEMICOLON){ 
	// 	//一个单独的赋值或条件表达式自成一行，
	// 	//并没有任何作用，后续不会被弹出，所以此处需要弹栈
	// 	// stack pop;
	// }
	// else {
	// 	// if(exp1 is lvalue){
	// 	// 		code_.gen(LODA,0,0);
	// 	// } 
	// }
}

void Parser::setjmp(symset fsys)
{
	// void expression(symset);
	getsym();
	if(Lex::sym() == SYM_LPAREN)
	{
		getsym();
		// expression(fsys);
		assign_expression(fsys);
		if(Lex::sym() == SYM_RPAREN)
		{
			getsym();
			code_.gen(SJP,0,0);
		}
		else{
			// error: missing ')'
		}
	}
	else{
		// error: expecting '('
	}
}


void Parser::longjmp(symset fsys)
{
	// void expression(symset);
	getsym();
	if(Lex::sym() == SYM_LPAREN)
	{
		getsym();
		// expression(fsys);
		assign_expression(fsys);
		getsym();
		// expression(fsys);
		assign_expression(fsys);
		if(Lex::sym() == SYM_RPAREN)
		{
			getsym();
			code_.gen(LJP,0,0);
		}
		else{
			// error: missing ')'
		}
	}
	else{
		// error: missing '('
	}
}

// struct Node{
// 	int used;
// 	int in0;
// 	int in1;
// };

// struct Node nodeList[gMaxInstNum];
// int optimizeStack[gMaxInstNum];
// instruction codeBuf[gMaxInstNum];
// void optimize(int begin,int end){

// 	// int* parent =(int*)malloc(sizeof(int)*(end-begin));
// 	int top=-1;
// 	for(int idx=begin;idx<end;idx++){
// 		instruction i=code[idx];
// 		nodeList[idx].used=0;
// 		nodeList[idx].in0=nodeList[idx].in1=-1;
// 		switch (i.funcCode)
// 		{
// 		case LIT:
// 		case LOD:
// 		case LEA:
// 			optimizeStack[++top]=idx;		
// 			break;
// 		case OPR:
// 			switch (i.address) // operator
// 			{
// 			case OPR_NEG:
// 				nodeList[idx].in0=optimizeStack[top];
// 				optimizeStack[top]=idx;
// 				break;
// 			case OPR_ADD:
// 			case OPR_MIN:
// 			case OPR_MUL:
// 			case OPR_DIV:
// 			case OPR_EQU:
// 			case OPR_NEQ:
// 			case OPR_LES:
// 			case OPR_GEQ:
// 			case OPR_GTR:
// 			case OPR_LEQ:
// 				nodeList[idx].in0=optimizeStack[top];
// 				nodeList[idx].in1=optimizeStack[top-1];
// 				optimizeStack[--top]=idx;
// 				break;
// 			} // switch
// 			break;
// 		case SJP:
// 		case LODA:
// 			nodeList[idx].in0=optimizeStack[top];
// 			optimizeStack[top]=idx;
// 			break;
// 		case STO:
// 			nodeList[idx].in0=optimizeStack[top];
// 			top--;
// 			break;
// 		case STOA:
// 			if(i.address==0){
// 				nodeList[idx].in0=optimizeStack[top--];
// 				nodeList[idx].in1=optimizeStack[top--];
// 			}
// 			else{
// 				nodeList[idx].in0=optimizeStack[top];
// 				nodeList[idx].in1=optimizeStack[top-1];
// 				optimizeStack[--top]=idx;
// 			}
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 	for(int idx=end-1;idx>=begin;idx--){
// 		struct Node n=nodeList[idx];
// 		instruction i=code[idx];
// 		if(n.used){
// 			nodeList[idx].used=1;
// 			if(n.in0!=-1)nodeList[n.in0].used=1;
// 			if(n.in1!=-1)nodeList[n.in1].used=1;
// 		}
// 		else{
// 			if(i.funcCode==STOA){
// 				code[idx].address=0;
// 				nodeList[idx].used=1;
// 				if(n.in0!=-1)nodeList[n.in0].used=1;
// 				if(n.in1!=-1)nodeList[n.in1].used=1;
// 			}
// 			else if(i.funcCode==STO){
// 				nodeList[idx].used=1;
// 				if(n.in0!=-1)nodeList[n.in0].used=1;
// 				if(n.in1!=-1)nodeList[n.in1].used=1;
// 			}
// 			else if(i.funcCode==SJP){
// 				code[idx].address=1;
// 				nodeList[idx].used=1;
// 				if(n.in0!=-1)nodeList[n.in0].used=1;
// 				if(n.in1!=-1)nodeList[n.in1].used=1;
// 			}
// 		}
// 	}
// 	int usednum=0;
// 	for(int idx=begin;idx<end;idx++){
// 		if(nodeList[idx].used){
// 			code[begin+usednum]=code[idx];
// 			usednum++;
// 		}
// 	}
// 	code_.codeIndex()=begin+usednum;
	
// }


//////////////////////////////////////////////////////////////////////
void Parser::condition(symset fsys)
{
	int relop;
	symset set;

	if (Lex::sym() == SYM_ODD)
	{
		getsym();
		// expression(fsys);
		assign_expression(fsys);
		code_.gen(OPR, 0, 6);
	}
	else
	{
		set = uniteset(relset, fsys);
		// expression(set);
		assign_expression(set);
		destroyset(set);
		if (! inset(Lex::sym(), relset))
		{
			error(20);
		}
		else
		{
			relop = Lex::sym();
			getsym();
			// expression(fsys);
			assign_expression(fsys);
			switch (relop)
			{
			case SYM_EQU:
				code_.gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				code_.gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				code_.gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				code_.gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				code_.gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				code_.gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // else
	} // else
} // condition

void Parser::paralist_(symset fsys,int* len){
	if(Lex::sym()==SYM_COMMA){
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

void Parser::paralist(symset fsys,int* len){
	// expression(fsys);
	assign_expression(fsys);
	int paralist_len;
	paralist_(fsys,&paralist_len);
	*len=paralist_len+1;

}


void Parser::Elist_(symset fsys,ArrayInf& array,int ndim){
	if(Lex::sym()==SYM_COMMA){// ,
		if(ndim>=array.dim()){
			// printf("ndim > array.dim\n");
			//error; array have array.dim dims,but get ndim dims
		}
		getsym();
		// expression(fsys);
		assign_expression(fsys);
		code_.gen(LIT,0,array.limit(ndim));
		code_.gen(OPR,0,OPR_MUL);
		Elist_(fsys,array,ndim+1);
		code_.gen(OPR,0,OPR_ADD);
	}
	else if(ndim<array.dim()-1){
		// printf("ndim < array.dim-1\n");
		//error; array have array.dim dims,but get ndim dims
	}
	else{
		code_.gen(LIT,0,0);
	}
}

void Parser::Elist(symset fsys,ArrayInf& array){
	// expression(fsys);
	assign_expression(fsys);
	// printf("%d\n",Lex::sym());
	//place*limit
	code_.gen(LIT,0,array.limit(0));
	code_.gen(OPR,0,OPR_MUL);
	Elist_(fsys,array,1);
	code_.gen(OPR,0,OPR_ADD);
}

void Parser::V_(symset fsys,ArrayInf& array){
	if(Lex::sym()==SYM_LBRACKET){
		getsym();
		Elist(fsys,array);
		if(Lex::sym()!=SYM_RBRACKET){
			//error expect ]
			// printf("expect ]\n");
		}
		getsym();
	}
	else{
		code_.gen(LIT,0,0);
	}
	return;
}

void Parser::V(symset fsys){
	if(Lex::sym()!=SYM_IDENTIFIER){
		//error expect Lex::id()
	}
	int i=idTable_.position(Lex::id());
	if(i==-1){
		error(11); //Undeclared identifier.
	}
	else if(!(idTable_.getKind(i)==ID_VARIABLE||idTable_.getKind(i)==ID_ARRAY)){
		// printf("%d",table[i].kind);
		error(12); //Illegal assignment.
		i=0;
	}
	// mask* mk=(mask*)&table[i];
	getsym();
	if(Lex::sym()==SYM_LBRACKET&&idTable_.getKind(i)!=ID_ARRAY){
		//error; Lex::id() is not array
	}
	else if(Lex::sym()!=SYM_LBRACKET&&idTable_.getKind(i)==ID_ARRAY){
		//error; expect [
	}
    ArrayInf array=idTable_.getArrayInf(i);
	V_(fsys,array);
	code_.gen(LEA,level_-idTable_.getLevel(i),idTable_.getAddress(i));
	code_.gen(OPR,0,OPR_ADD);
}

// void low(int i){
// 	if(Lex::sym()==SYM_NUMBER){
// 		getsym();
// 		code_.gen(LIT,0,Lex::num());
// 	}
// 	else if(Lex::sym()==SYM_CONST){
		
// 	}
// 	else {
// 		//error expect const or number
// 	}
// }

// void high(){

// }

// void step(){

// }

void Parser::rangeList(symset fsys){
	if(Lex::sym()==SYM_LPAREN){
		getsym();

		// low


		if(Lex::sym()==SYM_COMMA){
			getsym();
			//high

			if(Lex::sym()==SYM_COMMA){
				getsym();
				//step
				if(Lex::sym()==SYM_RPAREN){
					getsym();
				}
				else {
					//error expect )
				}
			}
			else if(Lex::sym()==SYM_RPAREN){
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

Parser::rangeRetVal Parser::range(symset fsys){
	int low=1,high=1,step=1,var_pos;
	bool low_minus=0,high_minus=0,step_minus=0;
	if(Lex::sym()==SYM_LPAREN){
		getsym();
		if(Lex::sym()==SYM_MINUS){
			getsym();
			low_minus=1;
		}
		if(Lex::sym()==SYM_NUMBER||Lex::sym()==SYM_CONST){
			getsym();
			if(Lex::sym()==SYM_CONST){
				int i=idTable_.position(Lex::id());
                if(i==-1){
                    error(11);
                }
				low=idTable_.getVal(i);
			}
			else {
				low=Lex::num();
			}
			if(Lex::sym()==SYM_COMMA){
				getsym();
				if(Lex::sym()==SYM_MINUS){
					getsym();
					high_minus=1;
				}
				if(Lex::sym()==SYM_NUMBER||Lex::sym()==SYM_CONST){
					getsym();
					if(Lex::sym()==SYM_CONST){
						int i=idTable_.position(Lex::id());
                        if(i==-1){
                            error(11);
                        }
						high=idTable_.getVal(i);
					}
					else {
						high=Lex::num();
					}
					if(Lex::sym()==SYM_COMMA){
						getsym();
						if(Lex::sym()==SYM_MINUS){
							getsym();
							step_minus=1;
						}
						if(Lex::sym()==SYM_NUMBER||Lex::sym()==SYM_CONST){
							//step
							getsym();
							if(Lex::sym()==SYM_CONST){
								int i=idTable_.position(Lex::id());
                                if(i==-1){
                                    error(11);
                                }
								step=idTable_.getVal(i);
							}
							else {
								step=Lex::num();
							}
							if(Lex::sym()==SYM_RPAREN){
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
					else if(Lex::sym()==SYM_RPAREN){
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
	rangeRetVal ret={low_minus?-low:low,high_minus?-high:high,step_minus?-step:step};
	return ret;
}

void Parser::for_statement(symset fsys){
	// void statement(symset fsys);
	int cx1,cx2;
	if(Lex::sym()==SYM_FOR){
		getsym();
		if(Lex::sym()==SYM_LPAREN){
			getsym();
			if(Lex::sym()==SYM_VAR){
				getsym();
				if(Lex::sym()==SYM_IDENTIFIER){
					getsym();
					// char oldId[gMaxIdLen+1];
                    std::string oldId=Lex::id();
					// mask* mk;
					// strcpy(oldId,Lex::id());
					idTable_.enter(ID_VARIABLE,NULL,Lex::id(),level_);
					// mk=(mask*)&table[tableIndex];
                    int i=idTable_.tableIndex();
					if(Lex::sym()==SYM_COLON){
						getsym();
						rangeRetVal ret=range(fsys);
						if(Lex::sym()==SYM_RPAREN){
							getsym();
							// i=low
							code_.gen(LIT,0,ret.low);
							code_.gen(STO,level_-idTable_.getLevel(i),idTable_.getAddress(i));
							cx1=code_.codeIndex();
                            
							// if not i<=high or i>=high jmp
							code_.gen(LOD,level_-idTable_.getLevel(i),idTable_.getAddress(i));
							code_.gen(LIT,0,ret.high);
							if(ret.step>0)code_.gen(OPR,0,OPR_LEQ);
							else if(ret.step<0)code_.gen(OPR,0,OPR_GEQ);
							else {
								error(29);
								code_.gen(OPR,0,0);
							}
							cx2=code_.codeIndex();
							code_.gen(JPC,0,0);
							statement(fsys);
							// i=i+step
							code_.gen(LOD,level_-idTable_.getLevel(i),idTable_.getAddress(i));
							code_.gen(LIT,0,ret.step);
							code_.gen(OPR,0,OPR_ADD);
							code_.gen(STO,level_-idTable_.getLevel(i),idTable_.getAddress(i));
							//jmp
							code_.gen(JMP,0,cx1);
							// code[cx2].address=code_.codeIndex();
							code_.backFill(cx2,code_.codeIndex());
							// tableIndex--;
							idTable_.setTableIndex(idTable_.tableIndex()-1);

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
// int last_cx1;
void Parser::statement(symset fsys)
{
	int i,cx1,cx2;
	symset set1, set;

	// if (Lex::sym() == SYM_IDENTIFIER)
	// { // variable assignment
	// 	mask* mk;
	// 	if (! (i = idTable_.hasId(Lex::id())))
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
	// 	if (Lex::sym() == SYM_BECOMES)
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
	// 			code_.gen(STO, level_ - idTable_.getLevel(i), idTable_.getAddr(i));
	// 		}
	// 		else if (table[i].kind==ID_ARRAY){
	// 			code_.gen(STOA,level_-idTable_.getLevel(i),0);
	// 		}
	// 	}

	// }
	symset assignExpBeginSet=createset(SYM_IDENTIFIER,SYM_NUMBER,SYM_MINUS,SYM_CONST,SYM_LPAREN);
	if (inset(Lex::sym(),assignExpBeginSet))
	{ // variable assignment
		int begincx=code_.codeIndex();
		assign_expression(fsys);
		code_.optimize(begincx,code_.codeIndex());

        
	}
	else if (Lex::sym() == SYM_CALL)
	{ // procedure call
		getsym();
		if (Lex::sym() != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if ((i = idTable_.position(Lex::id()))==-1)
			{
				error(11); // Undeclared identifier.
			}
			else if (idTable_.getKind(i)== ID_PROCEDURE)
			{
				// mask* mk;
				// mk = (mask*) &table[i];
				code_.gen(CAL, level_ - idTable_.getLevel(i), idTable_.getAddress(i));
			}
			else
			{
				error(15); // A constant or variable can not be called. 
			}
			getsym();
		}
	} 
	else if (Lex::sym() == SYM_IF)
	{ // if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		// condition(set);
		assign_expression(set);
		destroyset(set1);
		destroyset(set);
		if (Lex::sym() == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}
		cx1 = code_.codeIndex();
		code_.gen(JPC, 0, 0);
		statement(fsys);
		last_cx1=cx1;
		// code[cx1].address = code_.codeIndex();
		code_.backFill(cx1,code_.codeIndex());
		test(fsys, phi, 19);
		return ;
	}
	else if(Lex::sym() == SYM_ELSE){
		if(last_cx1==-1){
			//error no matched if
		}
		getsym();
		cx2=code_.codeIndex();
		code_.gen(JMP,0,0);
		// code[last_cx1].address=code_.codeIndex();
		code_.backFill(last_cx1,code_.codeIndex());
		statement(fsys);
		// code[cx2].address=code_.codeIndex();
		code_.backFill(last_cx1,code_.codeIndex());
	}
	else if (Lex::sym() == SYM_BEGIN)
	{ // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (Lex::sym() == SYM_SEMICOLON || inset(Lex::sym(), statbegsys))
		{
			if (Lex::sym() == SYM_SEMICOLON)
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
		if (Lex::sym() == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (Lex::sym() == SYM_WHILE)
	{ // while statement
		cx1 = code_.codeIndex();
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		// condition(set);
		assign_expression(set);
		destroyset(set1);
		destroyset(set);
		cx2 = code_.codeIndex();
		code_.gen(JPC, 0, 0);
		if (Lex::sym() == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		statement(fsys);
		code_.gen(JMP, 0, cx1);
		// code[cx2].address = code_.codeIndex();
		code_.backFill(cx2,code_.codeIndex());
	}
	else if(Lex::sym() == SYM_PRINT){
		getsym();
		if(Lex::sym()==SYM_LPAREN){
			getsym();
			if(Lex::sym()==SYM_RPAREN){
				getsym();
				code_.gen(PRT,0,0);
			}
			else{
				int len;
				paralist(fsys,&len);
				if(Lex::sym()==SYM_RPAREN){
					getsym();
					code_.gen(PRT,0,len);
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
	else if(Lex::sym()==SYM_FOR){
		for_statement(fsys);
	}
	else if(Lex::sym()==SYM_LONGJMP){
		longjmp(fsys);
	}
	test(fsys, phi, 19);
	// last_cx1=-1;
} // statement



//////////////////////////////////////////////////////////////////////
void Parser::block(symset fsys)
{
	int cx0; // initial code index
	// mask* mk;
	int block_dx;
	int savedTx;
	symset set1, set;

	// dx = 3;
    idTable_.setDx(3);
	block_dx = idTable_.dx();
	// mk = (mask*) &table[tableIndex];
    int i=idTable_.tableIndex();
	idTable_.setProcAddr(i,code_.codeIndex());
	// printf("addr----%d",idTable_.getAddress(i));
	// mk->address = code_.codeIndex();
	code_.gen(JMP, 0, 0);
	if (level_ > gMaxLevel)
	{
		error(32); // There are too many levels.
	}
	do
	{
		// printf("%d\n",Lex::sym());
		if (Lex::sym() == SYM_CONST)
		{ // constant declarations
			getsym();
			do
			{
				constdeclaration();
				while (Lex::sym() == SYM_COMMA)
				{
					getsym();
					constdeclaration();
				}
				if (Lex::sym() == SYM_SEMICOLON) //;
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (Lex::sym() == SYM_IDENTIFIER);
		} // if

		if (Lex::sym() == SYM_VAR)
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();
				while (Lex::sym() == SYM_COMMA)
				{
					getsym();
					vardeclaration();
				}
				if (Lex::sym() == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (Lex::sym() == SYM_IDENTIFIER);
		} // if
		block_dx = idTable_.dx(); //save dx before handling procedure call!
		while (Lex::sym() == SYM_PROCEDURE)
		{ // procedure declarations
			getsym();
			if (Lex::sym() == SYM_IDENTIFIER)
			{
				idTable_.enter(ID_PROCEDURE,NULL,Lex::id(),level_);
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}


			if (Lex::sym() == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}

			level_++;
			savedTx = idTable_.tableIndex();
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);
			destroyset(set1);
			destroyset(set);
			// tableIndex = savedTx;
			idTable_.setTableIndex(savedTx);
			level_--;

			if (Lex::sym() == SYM_SEMICOLON)
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
		idTable_.setDx(block_dx); //restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	}
	while (inset(Lex::sym(), declbegsys));

	// code[mk->address].address = code_.codeIndex();
	code_.backFill(idTable_.getAddress(i),code_.codeIndex());
	// mk->address = code_.codeIndex();
	idTable_.setProcAddr(i,code_.codeIndex());
	cx0 = code_.codeIndex();
	code_.gen(INT, 0, block_dx);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	code_.gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	// listcode(cx0, code_.codeIndex());
} // block

 void Parser::parse(){
        symset set, set1, set2;
        // getsym();
        phi = createset(SYM_NULL);
        relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);
        
        // create begin symbol sets
        declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
        statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE,SYM_FOR,SYM_ELSE,SYM_LONGJMP,SYM_NULL);
        facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS,SYM_SETJMP, SYM_NULL);

        // kk = gMaxIdLen;

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
        if(Lex::sym()!=SYM_PERIOD)
            error(9);//'.' expected
        if(gErr==0){
            code_.write("hbin.txt");
        }
    }