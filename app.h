#ifndef APP
#define APP

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "basic.h"

extern int mode_of_sys;
extern int mode_of_work;
extern unsigned int host;
extern unsigned short port;
extern int client;

extern struct context send_ctx;
extern struct context recv_ctx;

bool init_connection(void);
bool send_msg(char *msg, int len);
bool recv_msg(char *msg, int *plen);
bool send_data(byte *loc, int len, int flag);
bool recv_data(byte *loc, int len, int flag);
bool reset_connection(void);
bool close_connection(void);

void setup_context(struct context *ctx, char *key, byte *IV);

char *key = "123456";
byte challenge[16] = "\x58\x90\xAE\x86\xF1\xB9\x1C\xF6" \
	"\x29\x83\x95\x71\x1D\xDE\x58\x0D";

#endif /* app.h */
