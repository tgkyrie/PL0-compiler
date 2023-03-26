#include"Lex.h"
#include"error.h"
#include<iostream>

char Lex::chLastRead_=' ';
int Lex::symLastRead_=0;
std::string Lex::idLastRead_="";
int Lex::numLastRead_=0;
int Lex::chCount_=0;
std::string Lex::line_="";

void Lex::getch(void)
{
	if (chCount_ >= line_.size())
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
        line_="";
		chCount_ = 0;
		// printf("%5d  ", codeIndex);
		while ( (!feof(infile)) // added & modified by alex 01-02-09
			    && ((chLastRead_ = getc(infile)) != '\n'))
		{
			printf("%c", chLastRead_);
			// line_[++lineLen_] = chLastRead_;
            line_+=chLastRead_;
		} // while
		printf("\n");
		// line_[++lineLen] = ' ';
        line_+=' ';
	}
	chLastRead_ = line_[chCount_++];
} // getch

void Lex::getsym()
{
	int i, k;
	// char a[gMaxIdLen + 1];
    std::string a="";
	while (chLastRead_ == ' '||chLastRead_ == '\t')
		getch();

	if (isalpha(chLastRead_))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < gMaxIdLen)
				// a[k++] = chLastRead_;
				a+=chLastRead_;
			getch();
		}
		while (isalpha(chLastRead_) || isdigit(chLastRead_));
		// a[k] = 0;
		// strcpy(idLastRead_, a);
		// strcpy()
        // a=idLastRead_;
		idLastRead_=a;
		// std::cout<<idLastRead_<<std::endl;
		reservedWords[0] = idLastRead_;
		i = gReservedWordsNum;
		while (idLastRead_!=reservedWords[i--]);
		if (++i)
			symLastRead_ = reservedWordsSym[i]; // symbol is a reserved word
		else
			symLastRead_ = SYM_IDENTIFIER;   // symbol is an identifier
	}
	else if (isdigit(chLastRead_))
	{ // symbol is a number.
		k = numLastRead_ = 0;
		symLastRead_ = SYM_NUMBER;
		do
		{
			numLastRead_ = numLastRead_ * 10 + chLastRead_ - '0';
			k++;
			getch();
		}
		while (isdigit(chLastRead_));
		if (k > gMaxNumLen)
			error(25);     // The number is too great.
	}
	else if (chLastRead_ == ':')
	{
		getch();
		if (chLastRead_ == '=')
		{
			symLastRead_ = SYM_BECOMES; // :=
			getch();
		}
		else
		{
			symLastRead_ = SYM_COLON;       // illegal?
		}
	}
	else if (chLastRead_ == '>')
	{
		getch();
		if (chLastRead_ == '=')
		{
			symLastRead_ = SYM_GEQ;     // >=
			getch();
		}
		else
		{
			symLastRead_ = SYM_GTR;     // >
		}
	}
	else if (chLastRead_ == '<')
	{
		getch();
		if (chLastRead_ == '=')
		{
			symLastRead_ = SYM_LEQ;     // <=
			getch();
		}
		else if (chLastRead_ == '>')
		{
			symLastRead_ = SYM_NEQ;     // <>
			getch();
		}
		else
		{
			symLastRead_ = SYM_LES;     // <
		}
	}
	else
	{ // other tokens
		i = gNSym;
		reservedChar[0] = chLastRead_;
		while (reservedChar[i--] != chLastRead_);
		if (++i)
		{
			symLastRead_ = reservedCharSym[i];
			getch();
		}
		else
		{
			printf("%d\n",(int)(chLastRead_));
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
	// printf("sym:%d\n",symLastRead_);
} // getsym