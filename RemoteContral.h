#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#define FORWARDCON 0
#define REVERSECON 1
#define BUFSIZE 105	//to be perfected
#define GET_FILE 0
#define PUT_FILE 1
#define RUN_SHELL 2
