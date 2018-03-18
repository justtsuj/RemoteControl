#ifndef REMOTE_CONTROL
#define REMOTE_CONTROL

#include <stdbool.h>
#include <sys/stat.h>
#define CLIENT 0
#define SERVER 1
#define FORWARDCON 0
#define REVERSECON 1
#define BUFSIZE 100	//to be perfected
#define GET_FILE 0
#define PUT_FILE 1
#define RUN_SHELL 2
#define FAILURE false

bool init_connection(void);
bool send_msg(char *msg, int len);
bool recv_msg(char *msg, int *plen);
bool reset_connection(void);
bool close_connection(void);

#endif
