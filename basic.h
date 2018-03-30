#ifndef BASIC
#define BASIC

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

typedef unsigned char byte;
typedef unsigned long int word;

#define CLIENT 0
#define SERVER 1
#define FORWARDCON 0
#define REVERSECON 1
#define BUFSIZE 100	//to be perfected
#define GET_FILE 0
#define PUT_FILE 1
#define RUN_SHELL 2
#define FAILURE false


#endif /* basic.h */
