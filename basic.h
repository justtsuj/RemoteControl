#ifndef BASIC
#define BASIC

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

typedef unsigned char byte;
typedef unsigned long int word;

#define CLIENT 0
#define SERVER 1
#define FORWARDCON 0
#define REVERSECON 1
#define MSGSIZE 1000	//to be perfected
#define GET_FILE 0
#define PUT_FILE 1
#define RUN_SHELL 2
#define SYSINFO 3
#define FAILURE false
#define BUFSIZE (MSGSIZE + 2 + 16 + 20)
#define SERVERPORT 7586
#define FAKENAME "/bin/bash"

#define BACKGROUND
#define REVERSE
//#define DEBUG
//#define EXIT goto catch
#define EXIT return ret

#endif /* basic.h */
