#pragma once
#include"ArrayInf.h"
// #include"PL0.h"
#include"error.h"
#include"set.h"
#include"Lex.h"
#include"IdTable.h"
#include"Instructions.h"
class Parser
{
private:
    /* data */
    int level_=0;
    Lex lex_;
    IdTable idTable_;
    int last_cx1;
    Instructions code_;
    void test(symset s1, symset s2, int n);
    void getsym() {lex_.getsym();}
    void constdeclaration();
    void brackets(ArrayInf& array);
    void vardeclaration();
    bool factor(symset fsys,bool inferNotLvalue);
    bool term(symset fsys,bool inferNotLvalue);
    bool expression(symset fsys,bool inferNotLvalue);
    bool condition_expression(symset fsys);
    void assign_expression_(symset fsys);
    bool assign_expression(symset fsys);
    void setjmp(symset fsys);
    void longjmp(symset fsys);
    void condition(symset fsys);
    void paralist_(symset fsys,int* len);
    void paralist(symset fsys,int* len);
    void Elist_(symset fsys,ArrayInf& array,int ndim);
    void Elist(symset fsys,ArrayInf& array);
    void V_(symset fsys,ArrayInf& array);
    void V(symset fsys);
    void rangeList(symset fsys);
    typedef struct {
        int low;
        int high;
        int step;
    } rangeRetVal;
    rangeRetVal range(symset fsys);
    void for_statement(symset fsys);
    void statement(symset fsys);
    void block(symset fsys);

public:
    Parser():idTable_(gMaxIdTableLen),code_(gMaxInstNum){}
    void parse();
};

