#ifndef STACK_H
#define STACK_H

#include <stdio.h>

#ifndef BUFLEN
#define BUFLEN 512  //Max length of buffer
#endif

// Definition of static stack for i/o nonblocking reading
typedef struct Stack{
  char buf[BUFLEN];
  size_t top;
} Stack;

Stack* newstack();
void closestack(Stack* stack);
void push(Stack* stack, char c);
void flush(Stack* stack, char* buf);

#endif
