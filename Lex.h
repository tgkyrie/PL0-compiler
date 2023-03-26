#pragma once
#include<string>
#include "PL0.h"
class Lex
{
private:
    /* data */
    static char chLastRead_;         // last character read
    static int  symLastRead_;        // last symbol read
    static std::string idLastRead_; // last identifier read
    static int  numLastRead_;    
    static int chCount_;
    static std::string line_; 
    static void getch();
public:
    Lex(/* args */){}
    ~Lex(){}
    static void getsym();
    static int chCount(){return chCount_;}
    static std::string id(){return idLastRead_;}
    static int sym(){return symLastRead_;}
    static int num(){return numLastRead_;}
    static void setNum(int num){numLastRead_=num;}
};

