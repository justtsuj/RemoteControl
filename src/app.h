#ifndef APP
#define APP

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include "basic.h"

extern int client;
extern int server;
extern struct context send_ctx;
extern struct context recv_ctx;

void handle_error(int ret);
int create_server_socket(void);
int send_msg(char *msg, int len);
int recv_msg(char *msg, int *plen);
bool send_data(byte *loc, int len, int flag);
bool recv_data(byte *loc, int len, int flag);
void setup_context(struct context *ctx, char *key, byte *IV);

unsigned int host = 0;
unsigned short port = 7586;
char *key = "123456";
byte challenge[16] = "\x58\x90\xAE\x86\xF1\xB9\x1C\xF6" \
	"\x29\x83\x95\x71\x1D\xDE\x58\x0D";

#endif /* app.h */
