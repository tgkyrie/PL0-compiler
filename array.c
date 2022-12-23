/**
 * @file array.c
 * @author tgkyrie (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include"PL0.h"
#include"set.c"

/**
S-> V := E
V -> id V_
V_ -> [Elist] | NULL
Elist -> expression Elist_
Elist_ -> , expression Elist_ | NULL

expression -> term expression_
expression_ -> (+|-) term expression_ | NULL
term -> factor term_
term_ -> (*|/) factor term_ | NULL
factor -> id factor_ | num | (expression) | -factor
factor_ -> V_ | NULL


 */

void Elist_(){
    if(sym==','){
        expression();
        Elist_();
    }
    else{
        return;
    }
}

void Elist(){
    expression();
    Elist_();
}

void V_(){
    if(sym=='['){
        Elist();
        getsym();
        if(sym==']'){
            
        }
        else{
            //error 
            //expect ']'
        }
    }
    else{
        return ;
    }
}

void V(){
    if(sym==SYM_IDENTIFIER){
        getsym();
        V_();
    }
    else{
        //error
    }
}