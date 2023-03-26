#pragma once
#include"ArrayInf.h"
#include<vector>
#include<iostream>
class IdTable
{
private:
    typedef struct
    {
        // char name[gMaxIdLen + 1];
        std::string name;
        int  kind;
        int  value;
        ArrayInf array;
    } comtab;
    typedef struct
    {
        std::string name;
        int   kind;
        short level;
        short address;
        ArrayInf array;
    } mask;
    std::vector<comtab> table_;
    int tableIndex_=0;
    int dx_=0;
    /* data */
public:
    IdTable(const int maxTableSize):table_(maxTableSize){}
    void enter(int kind,ArrayInf* array,std::string oldId,int level)
    {
        mask* mk;

        tableIndex_++;
        // strcpy(table_[tableIndex].name, oldId);
        table_[tableIndex_].name=oldId;
        table_[tableIndex_].kind = kind;
        switch (kind)
        {
        case ID_CONSTANT:
            if (Lex::num() > gMaxAddress)
            {
                error(25); // The number is too great.
                Lex::setNum(0);
            }
            table_[tableIndex_].value = Lex::num();
            break;
        case ID_VARIABLE:
            mk = (mask*) &table_[tableIndex_];
            mk->level = level;
            mk->address = dx_++;
            break;
        case ID_PROCEDURE:
            mk = (mask*) &table_[tableIndex_];
            mk->level = level;
            break;
        case ID_ARRAY:
            mk = (mask*) &table_[tableIndex_];
            mk->level = level;
            mk->address=dx_;
            mk->array=*array;
            // dx+=limit(*array,-1);
            dx_+=array->limit(-1);
        } // switch
        // tableIndex_++;
    } // enter
    int position(const std::string& id){
        for(int i=0;i<=tableIndex_;i++){
            if(table_[i].name==id)return i;
        }
        printf("not declare %s\n",id.c_str());
        return -1;//notfound
    }
    int getKind(const int i){
        return table_[i].kind;
    }
    int getVal(const int i){
        return table_[i].value;
    }
    ArrayInf getArrayInf(const int i){
        return table_[i].array;
    }

    int getKind(const std::string& id){
        for(auto& x:table_){
            if(x.name==id){
                return x.kind;
            }
        }
        return -1;//notfound
    }
    int getVal(const std::string& id){
        for(auto& x:table_){
            if(x.name==id){
                return x.value;
            }
        }
        return -1;//notfound
    }
    short getLevel(const int i){
        mask* mk=(mask*)&table_[i];
        return mk->level;
    }
    short getAddress(const int i){
        mask* mk=(mask*)&table_[i];
        return mk->address;
    }
    void setProcAddr(const int i,short addr){
        if(i>tableIndex_)return;
        // if(table_[i].kind!=ID_PROCEDURE)return;
        mask* mk=(mask*)&table_[i];
        mk->address=addr;
    }
    ArrayInf getArrayInf(const std::string& id){
        for(auto& x:table_){
            if(x.name==id){
                return x.array;
            }
        }
        // return 0;//notfound
    }
    int tableIndex()const{return tableIndex_;}
    void setTableIndex(int t){tableIndex_=t;}
    int dx()const{return dx_;}
    void setDx(int dx){
        dx_=dx;
    }
};

