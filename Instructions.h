#pragma once
#include"PL0.h"
#include<vector>
#include<iostream>
class Instructions
{
private:
    /* data */
    std::vector<instruction> code_;
    typedef struct Node{
        int used;
        int in0;
        int in1;
    } Node;
    // Node nodeList_[gMaxInstNum];
    std::vector<Node> nodeList_;
    std::vector<int> optimizeStack_;
    // int optimizeStack_[gMaxInstNum];
    // instruction codeBuf_[gMaxInstNum];
    // std::vector<instruction> codeBuf_;
public:
    Instructions(int maxInstNum):code_(0),
        nodeList_(maxInstNum),
        optimizeStack_(maxInstNum)
        // codeBuf_(maxInstNum)
        {
            code_.reserve(maxInstNum);
        }
    
    Instructions(const Instructions& that){
        code_=that.code_;
    }

    void gen(int func, int level, int address)
    {
        if (code_.size() >= code_.capacity())
        {
            printf("Fatal Error: Program too long.\n");
            exit(1);
        }
        instruction ins{func,level,address};
        code_.push_back(ins);
    } // gen
    int codeIndex()const {return code_.size();}

    void backFill(int index,int address){
        // printf("index----%d,addr----:%d\n",index,address);
        code_[index].address=address;
    }
    void write(const std::string& fileName){
        FILE* hbin = fopen(fileName.c_str(), "w");
		for (int i = 0; i < codeIndex(); i++)
			fwrite(&code_[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
        listcode();
    }
    void read(){
        instruction ins;
        while (!feof(infile))
        {
            fread(&ins,sizeof(instruction),1,infile);
            // std::cout<<ins.funcCode<<std::endl;
            code_.push_back(ins);
        }
        listcode();
    }
    void listcode()
    {
        int i;
        
        printf("\n");
        for (i = 0; i < code_.size(); i++)
        {
        
            printf("%5d %s\t%d\t%d\n", i, mnemonic[code_[i].funcCode].c_str(), code_[i].level, code_[i].address);
        }
        printf("\n");
    } // listcode

    void optimize(int begin,int end){
        // int* parent =(int*)malloc(sizeof(int)*(end-begin));
        // int begin=0;
        // int end=code_.size();
        int top=-1;
        for(int idx=begin;idx<end;idx++){
            instruction i=code_[idx];
            nodeList_[idx].used=0;
            nodeList_[idx].in0=nodeList_[idx].in1=-1;
            switch (i.funcCode)
            {
            case LIT:
            case LOD:
            case LEA:
                optimizeStack_[++top]=idx;		
                break;
            case OPR:
                switch (i.address) // operator
                {
                case OPR_NEG:
                    nodeList_[idx].in0=optimizeStack_[top];
                    optimizeStack_[top]=idx;
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
                    nodeList_[idx].in0=optimizeStack_[top];
                    nodeList_[idx].in1=optimizeStack_[top-1];
                    optimizeStack_[--top]=idx;
                    break;
                } // switch
                break;
            case SJP:
            case LODA:
                nodeList_[idx].in0=optimizeStack_[top];
                optimizeStack_[top]=idx;
                break;
            case STO:
                nodeList_[idx].in0=optimizeStack_[top];
                top--;
                break;
            case STOA:
                if(i.address==0){
                    nodeList_[idx].in0=optimizeStack_[top--];
                    nodeList_[idx].in1=optimizeStack_[top--];
                }
                else{
                    nodeList_[idx].in0=optimizeStack_[top];
                    nodeList_[idx].in1=optimizeStack_[top-1];
                    optimizeStack_[--top]=idx;
                }
                break;
            default:
                break;
            }
        }
        for(int idx=end-1;idx>=begin;idx--){
            Node n=nodeList_[idx];
            instruction i=code_[idx];
            if(n.used){
                nodeList_[idx].used=1;
                if(n.in0!=-1)nodeList_[n.in0].used=1;
                if(n.in1!=-1)nodeList_[n.in1].used=1;
            }
            else{
                if(i.funcCode==STOA){
                    code_[idx].address=0;
                    nodeList_[idx].used=1;
                    if(n.in0!=-1)nodeList_[n.in0].used=1;
                    if(n.in1!=-1)nodeList_[n.in1].used=1;
                }
                else if(i.funcCode==STO){
                    nodeList_[idx].used=1;
                    if(n.in0!=-1)nodeList_[n.in0].used=1;
                    if(n.in1!=-1)nodeList_[n.in1].used=1;
                }
                else if(i.funcCode==SJP){
                    code_[idx].address=1;
                    nodeList_[idx].used=1;
                    if(n.in0!=-1)nodeList_[n.in0].used=1;
                    if(n.in1!=-1)nodeList_[n.in1].used=1;
                }
            }
        }
        int usednum=0;
        for(int idx=begin;idx<end;idx++){
            if(nodeList_[idx].used){
                code_[begin+usednum]=code_[idx];
                usednum++;
            }
        }
        // codeIndex=begin+usednum;
        code_.resize(begin+usednum);
    }
    instruction operator[](int i){
        return code_[i];
    }
};

