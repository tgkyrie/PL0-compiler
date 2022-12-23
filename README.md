# PL0编译

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


### 1.3 拓展PL0指令

增加STOA和LODA间接寻址指令

### 1.4 代码增删
增:
+ limit(array,ndim)
+ V(),V_(),Elist(),Elist_(),brackets()
+ arrayinf.h

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
### 2.2 PRT 内置指令
PRT 0 a

打印从top-a+1开始的a个栈元素，并将这a个元素弹出
若a==0则打印换行符号


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

### 5.2 属性定义
factor:
+ lvalue (factor -> V {factor.lvalue=1} ; factor -> num {factor.lvalue=0} ; factor -> (assign_expression) {factor.lvalue=assign_expression.lvalue})
+ place (factor -> V {factor.place=V.address} ; factor -> num {factor.place=num.val} ; factor -> (assign_expression) {factor.place = assign_expression.place})


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



