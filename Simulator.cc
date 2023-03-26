#include"PL0.h"
#include"Instructions.h"
#include"Simulator.h"
#include<string.h>
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;
	
	while (levelDiff--)
		b = stack[b];
	return b;
} // base
void interpret()
{
	int pc;        // program counter
	int stack[gStackSize];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register
    Instructions code(gMaxInstNum);
    code.read();
	int buf_idx;
	int ljp_ret;
	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];
		switch (i.funcCode)
		{
		case LIT:
			stack[++top] = i.address;
			break;
		case OPR:
			switch (i.address) // operator
			{
			case OPR_RET:
				top = b - 1;
				pc = stack[top + 3];
				b = stack[top + 2];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
				break;
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
				break;
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
				break;
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.level) + i.address];
			break;
		case LODA:
			// stack[top]=stack[base(stack,b,i.level)+stack[top]];
			stack[top]=stack[stack[top]];
			break;
		case STO:
			stack[base(stack, b, i.level) + i.address] = stack[top];
			// printf("%d\n", stack[top]);
			top--;
			break;
		case STOA:
			// stack[base(stack,b,i.level)+stack[top-1]]=stack[top];
			stack[stack[top-1]]=stack[top];
			// printf("%d\n", stack[top]);
			if(i.address==0){
				top-=2;
			}
			else {
				stack[top-1]=stack[top];
				top--;
			}
			break;
		case LEA:
			stack[++top]=base(stack,b,i.level)+i.address;
			break;
		case CAL:
			stack[top + 1] = base(stack, b, i.level);
			// generate new block mark
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.address;
			break;
		case INT:
			top += i.address;
			break;
		case JMP:
			pc = i.address;
			break;
		case JPC:
			if (stack[top] == 0)
				pc = i.address;
			top--;
			break;
		case PRT:
			if(i.address==0){
				printf("\n");
			}
			else{
				for(int k=top-i.address+1;k<=top;k++){
					printf("%d ",stack[k]);
				}
				top-=i.address;
			}
			break;
		case SJP:
			buf_idx = (stack[top--] % 4) * 1024;
			jmp_buf[buf_idx + 1] = pc;
			jmp_buf[buf_idx + 2] = b;
			jmp_buf[buf_idx + 3] = top;
			jmp_buf[buf_idx + 4] = i.funcCode;
			jmp_buf[buf_idx + 5] = i.level;
			jmp_buf[buf_idx + 6] = i.address;
			memcpy(jmp_buf +buf_idx+ 7, stack + b, sizeof(int)*(top - b + 1));
			if(i.address==0){
				stack[++top] = 0;
			}
			break;
		case LJP:
			ljp_ret = (stack[top--]);
			buf_idx = (stack[top--] % 4) * 1024;
			pc = jmp_buf[buf_idx + 1];
			b = jmp_buf[buf_idx + 2];
			top = jmp_buf[buf_idx + 3];
			i.funcCode = jmp_buf[buf_idx + 4];
			i.level = jmp_buf[buf_idx + 5];
			i.address = jmp_buf[buf_idx + 6];
			memcpy(stack + b, jmp_buf + buf_idx+7, sizeof(int)*(top - b + 1));
			stack[++top] = ljp_ret;
			break;
		} // switch
	}
	while (pc);

	printf("\nEnd executing PL/0 program.\n");
} // interpret
