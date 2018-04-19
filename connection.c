#include "basic.h"
#include "connection.h"
#include "crypt.h"
#include "sha1.h"

byte buffer[BUFSIZE];    //send_data,recv_data buffer
int client, server;

extern struct context recv_ctx;

void setup_context(struct context *ctx, char *key, unsigned char IV[20]);
void encrypt(byte *data, int len);
bool decrypt(byte *data, int len);

//socket setting
int create_server_socket(){
	struct sockaddr_in serv_addr, clie_addr;
	int clie_addr_len;
	int opt=1;
	if((server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return 3;
	if(setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		return 5;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERVERPORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	if(bind(server, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		close(server);	//you ya
		return 6;
	}
	if(listen(server, 1) < 0){
		close(server);
		return 7;
	}
    	return 0;
}

//send a complete data
//include error handing
bool send_data(byte *loc, int len, int flags){
	int send_data_len;
	int cur = 0;
	while(cur < len){
		send_data_len = send(client, loc + cur, len - cur, flags);
		if(send_data_len <= 0)
			return false;
		cur += send_data_len;
	}
	return true;
}

int send_msg(char *msg, int len){
	int blk_len;
/*#ifdef DEBUG
	printf("send %d\n", len);
	for(int i = 0; i < len; ++i)
		if(msg[i] > 32 && msg[i] < 127)
			printf("%5c", msg[i]);
		else
			printf("%#5x", (byte)msg[i]);
	printf("\n");
#endif*/
#ifdef DEBUGCRYPT
	printf("send %d\n", len);
	printf("plaintext: \n");
	for(int i = 0; i < len; ++i)
		printf("%02x ", (byte)msg[i]);
	printf("\n");
#endif
	struct sha1_context sha1_ctx;
	if(len <= 0 || len >= MSGSIZE)	//redefine length
		return 23;
	buffer[0] = (len >> 8) & 0xff;
	buffer[1] = len & 0xff;
	memcpy(buffer + 2, msg, len);	//header file string.h
	blk_len = len + 2;
	if(blk_len & 0x0f)
		blk_len = (blk_len & 0xfffffff0) + 0x10;
	encrypt(buffer, blk_len);
#ifdef DEBUGCRYPT
	printf("ciphertext: \t");
	for(int i = 0; i < blk_len; ++i)
		printf("%02x ", buffer[i]);
	printf("\n");
#endif
	if(send_data(buffer, blk_len + 20, 0) == false)
		return 22;
	return 0;
}

bool recv_data(byte *loc, int len, int flags){
	int recv_msg_len;
	int cur = 0;
	while(cur < len){
		recv_msg_len = recv(client, loc + cur, len - cur, flags);
		if(recv_msg_len <= 0) return false;
		cur += recv_msg_len;
	}
	return true;
}

//set max message length
int recv_msg(char *msg, int *plen){
	int blk_len;
	int j;
	int ret;
	byte temp[0x10];
	*plen = 0;
	if(recv_data(buffer, 0x10, 0) == false){	
		return 20;
	}
	memcpy(temp, buffer, 0x10);
	aes_decrypt(&recv_ctx.SK, temp);
	for(j = 0; j < 0x10; ++j)
		temp[j] ^= recv_ctx.LCT[j];
	*plen = ((int)temp[0] << 8) + (int)temp[1];
	//printf("[+]: message length: %d\n", *plen);
	if(*plen <= 0 || *plen >= MSGSIZE){
		*plen = 0;
		return 21;
	}
	blk_len = *plen + 2;
	if(blk_len & 0x0f)
		blk_len = (blk_len & 0xfffffff0) + 0x10;
	if(recv_data(buffer + 0x10, blk_len - 0x10 + 20, 0) == false){
		*plen = 0;
		return 20;
	}
#ifdef DEBUGCRYPT
	printf("recv %d\n", *plen);
	printf("ciphertext: \t");
	for(int i = 0; i < blk_len; ++i)
		printf("%02x ", buffer[i]);
	printf("\n");
#endif
	if(decrypt(buffer, blk_len + 20) == FAILURE) return false;
	memcpy(msg, buffer + 2, *plen);
#ifdef DEBUGCRYPT
	printf("plaintext: \t");
	for(int i = 0; i < *plen; ++i)
		printf("%02x ", (byte)msg[i]);
	printf("\n");
#endif
/*#ifdef DEBUG
	printf("recv %d\n", *plen);
	for(int i = 0; i < *plen; ++i)
		if(msg[i] > 32 && msg[i] < 127)
			printf("%5c", msg[i]);
		else
			printf("%#5x", (byte)msg[i]);
	printf("\n");
#endif*/
	return 0;
}
