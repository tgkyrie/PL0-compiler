# PL0
## 该项目采用c++重构代码，集成了pl0编译器和pl0模拟器，代码易读
## 编译运行
```
mkdir build
cd build
cmake ..
make
```

```
使用 ./pl0 -h查看帮助
./pl0 -c filename [compile file]
./pl0 -r filename [run compiled file]
```

## 目录
- [PL0](#pl0)
  - [该项目采用c++重构代码，集成了pl0编译器和pl0模拟器，代码易读](#该项目采用c重构代码集成了pl0编译器和pl0模拟器代码易读)
  - [编译运行](#编译运行)
  - [目录](#目录)
  - [1.数组](#1数组)
    - [1.1 数组声明文法](#11-数组声明文法)
    - [1.2 赋值文法](#12-赋值文法)
    - [1.3 文法分析](#13-文法分析)
      - [Elist](#elist)
      - [Elist\_](#elist_)
      - [V](#v)
  - [2. print](#2-print)
    - [2.1 print文法](#21-print文法)
    - [2.2 文法分析](#22-文法分析)
    - [2.3 PRT 内置指令](#23-prt-内置指令)
  - [3.for语句](#3for语句)
    - [3.1 文法](#31-文法)
    - [3.2 文法分析](#32-文法分析)
  - [4.else子句](#4else子句)
    - [4.1 文法](#41-文法)
    - [4.2 文法分析](#42-文法分析)
    - [4.3 额外工作：条件表达式](#43-额外工作条件表达式)
  - [5. 赋值表达式](#5-赋值表达式)
    - [5.1 赋值表达式文法](#51-赋值表达式文法)
    - [5.2 新增属性定义](#52-新增属性定义)
    - [5.3 实现](#53-实现)
    - [5.4 额外工作：代码优化](#54-额外工作代码优化)
  - [6. setjmp 和 longjmp](#6-setjmp-和-longjmp)
    - [6.1 功能](#61-功能)
    - [6.2 文法](#62-文法)
    - [6.3 具体做法](#63-具体做法)
  - [7. 测试](#7-测试)
    - [array.txt](#arraytxt)
    - [for1.txt](#for1txt)
    - [else.txt](#elsetxt)
    - [assign.txt](#assigntxt)
    - [sjp.txt](#sjptxt)
## 1.数组
数组在变量声明和statement中出现

### 1.1 数组声明文法
```
block -> var vardeclaration_list
vardeclaration_list -> vardeclaration,vardeclaration_list
vardeclaration -> id brackets
brackets -> [ const|num ] brackets | NULL
```
属性定义

vardeclaration:

综合属性:array

brackets:

综合属性
+ size (brackets -> [ const|num ] brackets中 , brackets.size= const|num.val * brackets1.size。在brackets -> NULL中 brackets.size=1)
+ array (brackets.array)


### 1.2 赋值文法

在statement中，LL1文法如下
```
statement -> V := expression
V -> id V_
V_ -> [Elist] | NULL
Elist -> expression Elist_
Elist_ -> ,expression Elist_ | NULL

expression -> term expression_
expression_ -> (+|-) term expression_ | NULL
term -> factor term_
term_ -> (*|/) factor term_ | NULL
factor -> V | num | (expression) | -factor
```

翻译方案中，expression,term不改变,factor仅需增加factor->id[Elist]的翻译方案。

属性定义如下

V_:

继承属性:
+ array (在V -> id V_中,V_继承id.array if id.kind == array)记录数组维度信息
  
综合属性:
+ place (在V_ -> [Elist]中，由Elist表示的偏移量)

Elist:

继承属性:
+ array (V_ -> [Elist]中，继承V_.array)
  
综合属性:
+ place (Elist -> expression Elist_中,Elist.place=expression.place*limit(Elist.array,0)+Elist_.place)

Elist_:

继承属性:
+ array (Elist -> expression Elist_中继承Elist.array)
+ ndim (维度信息，在Elist -> expression Elist_中,Elist_.ndim=1,在Elist_ -> ,expression Elist1_中,Elist1_.ndim=Elist_.ndim+1)

综合属性:
+ place (偏移量,Elist_ -> ,expression Elist1_中,Elist_.place=expression.place*limit(Elist_.array,Elist_.ndim)+Elist1_.ndim)


### 1.3 文法分析
#### Elist
```cpp
void Elist(symset fsys,arrayInf array){
	// expression(fsys);
	assign_expression(fsys);
	//place*limit
	gen(LIT,0,limit(array,0));
	gen(OPR,0,OPR_MUL);
	Elist_(fsys,array,1);
	gen(OPR,0,OPR_ADD);
}
```
Elist会从上一个调用它的函数中继承`array`，该参数记录了数组维度，然后分析第一个expr，expr产生的指令会将值放置于栈顶，分析expr结束后产生LIT和MUL指令，将偏移expr和数组维度权重相乘，比如a[5][5]的数组，分析a[1,2]的偏移是1*5+2，该过程就是计算1 * 5的，`limit`函数是获取这个相乘权重5的。

#### Elist_

```cpp
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
```
Elist_和Elist类似，也是会产生计算偏移的汇编指令。

#### V
```cpp
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
```
V最终将偏移和基址相加的指令，但不进行访存，只把变量地址放于栈顶，由于后续赋值表达式的左右值属性，访存在上层决定。

为实现间接访存，增加`LEA`,`STOA`,`LODA`指令，参考某一次作业。
```cpp
case LODA:
    //由于在LEA中用了base了，这里就不用base
    // stack[top]=stack[base(stack,b,i.l)+stack[top]];
    stack[top]=stack[stack[top]];
    break;
case STOA:
    //为赋值表达式的实现，STOA有两种，一种弹栈两次，一种弹栈一次，由i.a决定
    // stack[base(stack,b,i.l)+stack[top-1]]=stack[top];
    stack[stack[top-1]]=stack[top];
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
```

## 2. print

### 2.1 print文法
```
S -> print( paralist|NULL )
paralist -> expression paralist_
paralist_ -> ,expression paralist_ | NULL
```
属性定义:

paralist:
+ len(paralist -> expression paralist_中 , paralist.len=paralist1_.len+1)

paralist_:

综合属性:
+ len (paralist_ -> ,expression paralist1_ 中,paralist_.len=paralist1_.len+1 ; paralist_ -> NULL 中,paralist_.len=0)

### 2.2 文法分析
```cpp
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
```
### 2.3 PRT 内置指令
PRT 0 a

打印从top-a+1开始的a个栈元素，并将这a个元素弹出
若a==0则打印换行符号
```cpp
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
```

## 3.for语句
### 3.1 文法
```
statement -> for (var rangeList ) statement
rangeList -> id ：(low,high,step) | id:(low,high)
```
其中low,high,step都为常量

### 3.2 文法分析
文法分析代码太长，可看源码的`for_statement`函数，这里仅展示关键部分
```cpp
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
    //...
    }
}
```
rangeList分析会将low,high,step返回，然后参考产生条件跳转指令即可




## 4.else子句
### 4.1 文法
```
statement -> if condition then statement 
statement -> else statement
```
这里不使用课本的 stmn -> if ...  else 这种if else的文法，是因为pl0的`statement`不带分号，若要解决，那就是LL(2)文法了。所以这里else单独做statment。并且在语义分析中分析else是否有匹配的if，这种文法最后的实现和c语言的if else并没有差别

### 4.2 文法分析
```cpp
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
```
这里last_cx1是一全局变量，记录上个匹配的if该回填的cx，他的维护可看if的源码

### 4.3 额外工作：条件表达式
由于示例pl0中的if是带括号的，类似这种 if (conditon)，原有if实现无法满足这种需求，所以将conditon改成了condition_expr，其操作符有`<`,`>`等，优先级比`+`，`-`低，比后续的赋值`:=`高。

具体实现和expr，term，factor相似
```cpp
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
```


## 5. 赋值表达式

### 5.1 赋值表达式文法
```
assign_expression -> expression assign_expression_
assign_expression_ -> := expression assign_expression_ | NULL
expression -> term expression_
expression_ -> (+|-) term expression_ | NULL
term -> factor term_ | -term
term_ -> (*|/) factor term_ | NULL
factor -> V | num | (assign_expression) 
```
在上述文法中允许不能被赋值的表达式被赋值，这并不符合语法，所以将在语义分析中分析赋值符号左边是否是左值，若不是左值则报错

### 5.2 新增属性定义
factor:
+ lvalue (factor -> V {factor.lvalue=1} ; factor -> num {factor.lvalue=0} ; factor -> (assign_expression) {factor.lvalue=assign_expression.lvalue})

term:
+ lvalue (term -> factor term_ {term.lvalue=factor.lvalue&term_.NULL} ; term -> -term {term.lvalue=0} )

term_:
+ NULL (term_ -> (*|/) factor term_ {term_.NULL=0} ; term_ -> NULL {term_.NULL=1} )

expression:
+ lvalue  (类似term)
  
expression_:
+ lvalue  (类似term_)

assign_expression:
+ lvalue (assign_expression -> expression assign_expression_ { assign_expression.lvalue=expression.lvalue&assign_expression_.NULL })
+ valid (assign_expression -> expression assign_expression_ { assign_expression.valid=assign_expression_.NULL | expression.lvalue })

assign_expression_:
+ NULL (assign_expression_ -> := expression assign_expression_  {assign_expression_.NULL=0} ; assign_expression_ -> NULL {assign_expression_.NULL=1} )

### 5.3 实现
为了实现与C一致的赋值表达式，修改了许多函数,比如expr和term和factor都改成了返回值为bool，返回值表示分析到表达式的是不是左值，同时增加了参数`inferNotLvalue`,该参数用于提前告知分析的不是左值。具体看源码。

这里展示关键的assign_expr和assign_expr_的源码
```cpp
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
}
```

### 5.4 额外工作：代码优化
在C语言中
```c
1+(a=1);
```
这种代码只会编译有用的部分`a=1`,而1+a是不编译的

同时，在我们的实现中若一个表达式单独一行,如下
```
1+2;
```
pl0编译器产生的代码会把结果3放到栈顶，而这个3并没有被用到，若该语句出现在循环中则可能会栈溢出。

解决方案为使用`无用代码删除`，通过数据流分析找到无用汇编指令并删除，具体可看`optimize`函数

## 6. setjmp 和 longjmp 

### 6.1 功能

`setjmp`和`longjmp`实现了程序跨函数的跳转。其中，`longjmp`函数调用时调用`setjmp`函数的函数的活动记录一定没有被释放。

`setjmp`函数接受一个参数，把当前进程的状态保存到缓冲区中参数指定的地址。`setjmp`函数初次调用时返回`0`，从`longjmp`返回时返回指定的值（一般非零）。

`longjmp`函数接受两个参数，第一个对应`setjmp`函数，对应`setjmp`调用时的状态。第二个参数为回到`setjmp`保存的状态时表达式的值。

### 6.2 文法

得益于赋值表达式的实现，`setjmp`可以看作一个`factor`，而`longjmp`由于不会在调用处产生返回值，所以看作`statement`。

**factor** -> **setjmp**

**setjmp** -> setjmp ( **expression** )

**statement** -> **longjmp**

**longjmp** -> longjmp ( **expression** , **expression** )

### 6.3 具体做法

`setjmp`函数保存的状态包括：四个寄存器的值和调用`setjmp`函数的栈帧的局部量。

新增两个PL/0指令：`SJP`, `LJP`.

对于 `setjmp`，读取并弹出栈顶的数据，把寄存器和栈帧保存到缓冲区，之后把常数0压到栈顶。

对于 `longjmp`, 先两次匹配文法符号 `statement`,其结果分别对应 `longjmp` 的两个参数，再恢复寄存器的状态，接着把缓冲区对应的内容拷贝到栈中，最后把第二个参数，即返回值压栈。
```cpp
case SJP:
    buf_idx = (stack[top--] % 4) * 1024;
    jmp_buf[buf_idx + 1] = pc;
    jmp_buf[buf_idx + 2] = b;
    jmp_buf[buf_idx + 3] = top;
    jmp_buf[buf_idx + 4] = i.f;
    jmp_buf[buf_idx + 5] = i.l;
    jmp_buf[buf_idx + 6] = i.a;
    memcpy(jmp_buf +buf_idx+ 7, stack + b, sizeof(int)*(top - b + 1));
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
    memcpy(stack + b, jmp_buf + buf_idx+7, sizeof(int)*(top - b + 1));
    stack[++top] = ljp_ret;
    break;
```

## 7. 测试
我们准备了个测试用例+文档的1个测试用例，位于`./test`文件夹下
### array.txt
测试多维数组
```
var a[10][10],i,j;
begin
    i:=1;
    j:=2;
    a[i,j]:=999;
    print(a[i,j]);
end .
```

### for1.txt
测试for循环，打印fibonacci数列第1到第10个元素
```
var a[10];
begin
    a[0]:=1;
    a[1]:=2;
    print(a[0]);
    print();
    print(a[1]);
    print();
    for (var i:(2,9,1)) begin
        print(i);
        a[i]:=a[i-1]+a[i-2];
        print(a[i]);
        print();
    end;
end .
```

### else.txt
测试if else
```
var i;
begin
    i:=2;
    if i>1 then
        print(1);
    else if i>3 then
        print(2);
    else 
        print(3);
end .
```

### assign.txt
测试赋值表达式
```
var i,a,b,c;
begin
    1+(i:=1);
    print(i);
    a:=(b:=1+(c:=1));
    print(a);
    print(b);
    print(c);
end .
```
### sjp.txt
测试setjmp和longjmp
```
var i;
procedure p;
begin
  i:=2;
  print(i);
  print();
  longjmp(1, 8);
  i:=3;
end;

procedure q;
begin
  i:=100;
  
end;
begin
  i:=setjmp(1);
  print(i);
  if i = 0 then
    call p;
  call q;
end.
```



