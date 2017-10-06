#include "stack.h"

#include <string.h>
#include <malloc.h>

Stack* newstack(){
  Stack* new_stack = (Stack*) malloc(sizeof(Stack));
  memset(new_stack,0,sizeof(*new_stack));
  return new_stack;
}

void closestack(Stack* stack){
  free(stack);
}

void push(Stack* stack, char c){
  if (stack->top >= BUFLEN)
    fprintf(stderr, "push() error: buffer overflow" );
  else{
    stack->buf[stack->top]=c;
    stack->top ++;
  }
}

void flush(Stack* stack, char* buf){
  strcpy(buf,stack->buf);
  memset(stack,0,sizeof(*stack));
}
////////////////////////////////////////////////////
