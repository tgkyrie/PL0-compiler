#include<iostream>
#include"Parser.h"
#include"Simulator.h"
int main(int argc,char* argv[]){
    if(argc<3){
        std::cout<<"require more argument. use -h to check usage\n";
    }
    else if(argc>3){
        std::cout<<"too many argument. use -h to check usage\n";
    }
    std::string args=argv[1];
    bool compile=false;
    if(args=="-c")compile=true;
    else if(args=="-r")compile=false;
    else{
        std::string usage="-c filename [compile file]\n-r filename [run compiled file]\n";
        std::cout<<usage;
        return 0;
    }
    if ((infile = fopen(argv[2], "r")) == NULL)
	{
		printf("File %s can't be opened.\n", argv[1]);
		exit(1);
	}
    if(compile){
        Parser parser;
        parser.parse();
    }
    else{
        interpret();
    }

}