#pragma once

#include<string.h>
#include<malloc.h>
#include"PL0.h"
#ifndef NULL
#define NULL 0
#endif
typedef struct 
{
    int capacity;
	int dim;
	int* array;
} arrayInf;

void arrayInfInit(arrayInf* array){
    array->capacity=0;
    array->dim=0;
    array->array=NULL;
}

void arrayInfDestroy(arrayInf* array){
    free(array->array);
}



void push_back(arrayInf* array,int x){
    if(array->capacity==0){
        array->array=(int*)malloc(sizeof(int)*4);
        array->capacity=4;
        array->array[array->dim++]=x;
        return;
    }
    if(array->dim>=array->capacity){
        int* newArray=(int*)malloc(sizeof(int)*array->capacity*2);
        memcpy(newArray,array->array,sizeof(int)*array->capacity);
    }
    array->array[array->dim++]=x;
}