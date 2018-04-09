#include "basic.h"
#include "connection.h"
#include "crypt.h"
#include "sha1.h"

byte buffer[BUFSIZE + 20] = {0};    //send_data,recv_data buffer
int mode_of_sys;
int mode_of_work = FORWARDCON;
unsigned int host = 0;
unsigned short port = 7586;
int client, server;

extern struct context recv_ctx;

void setup_context(struct context *ctx, char *key, unsigned char IV[20]);
void encrypt(byte *data, int len);
bool decrypt(byte *data, int len);

//whether to close the socket when a connection fails
bool create_client_socket(){
	struct sockaddr_in addr;
	if((client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return false;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = host;
	addr.sin_port = htons(port);
	if(connect(client, (struct sockaddr*)&addr, sizeof(addr)) < 0){
		close(client);
		return false;
	}
	return true;
}

//socket setting
bool create_server_socket(){
	struct sockaddr_in serv_addr, clie_addr;
	int clie_addr_len;
	int opt=1;
	if((server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return false;
	if(setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		return false;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	if(bind(server, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		close(server);	//you ya
		return false;
	}
	if(listen(server, 5 ) < 0){
		close(server);
		return false;
	}
	//whether the output shoule be added
	if((client = accept(server, (struct sockaddr *)&clie_addr, &clie_addr_len)) < 0){
		close(server);
		return false;
	}
    	return true;
}



bool reset_server_socket(){
	struct sockaddr_in clie_addr;
	int clie_addr_len;
	close(client);
	//printf("here\n");
	if((client = accept(server, (struct sockaddr *)&clie_addr, &clie_addr_len)) < 0){
		close(server);
		return false;
	}
	//printf("here\n");
    	return true;
}

bool reset_client_socket(){
	close(client);
	return true;
}

bool close_server_socket(){
	close(client);
	close(server);
	return true;
}

bool close_client_socket(){
	close(client);
	return true;
}

bool close_connection(){
	if(mode_of_sys ^ mode_of_work)
		return close_server_socket();
	else
		return close_client_socket();
}

bool reset_connection(){
	if(mode_of_sys ^ mode_of_work)
		return reset_server_socket();
	else
		return reset_client_socket();
}

bool init_connection(){
	if(mode_of_sys ^ mode_of_work)
		return create_server_socket();
	else
		return create_client_socket();
}

//send a complete data
//include error handing
bool send_data(byte *loc, int len, int flags){
	int send_data_len;
	int cur = 0;
	while(cur < len){
		send_data_len = send(client, loc + cur, len - cur, flags);
		if(send_data_len < 0)
			return false;
		cur += send_data_len;
		//printf("send %d\n", cur);
	}
	return true;
}

bool send_msg(char *msg, int len){
	int blk_len;
#ifdef DEBUG
	printf("send %d\n", len);
	for(int i = 0; i < len; ++i)
		if(msg[i] > 32 && msg[i] < 127)
			printf("%5c", msg[i]);
		else
			printf("%#5x", (byte)msg[i]);
	printf("\n");
#endif
#ifdef DEBUGCRYPT
	printf("send %d\n", len);
	printf("plaintext: \t");
	for(int i = 0; i < len; ++i)
		printf("%02x ", (byte)msg[i]);
	printf("\n");
#endif
	struct sha1_context sha1_ctx;
	if(len <= 0 || len > MSGSIZE)	//redefine length
		return false;
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
	return send_data(buffer, blk_len + 20, 0);
}

int recv_data(byte *loc, int len, int flags){
	int recv_msg_len;
	int cur = 0;
	while(cur < len){
		recv_msg_len = recv(client, loc + cur, len - cur, flags);
		if(recv_msg_len <= 0)
			return recv_msg_len;
		cur += recv_msg_len;
		//printf("recv %d\n", cur);
	}
	return cur;
}

//set max message length
int recv_msg(char *msg, int *plen){
	int blk_len;
	int j;
	int ret;
	byte temp[0x10];
	if((ret = recv_data(buffer, 0x10, 0)) <= 0){
		//printf("%d\n", ret);	
		return ret;
	}
	memcpy(temp, buffer, 0x10);
	//for(int i = 0; i < 0x10; ++i)
		//printf("%02x ", temp[i]);
	//printf("\n");
	aes_decrypt(&recv_ctx.SK, temp);
	for(j = 0; j < 0x10; ++j)
		temp[j] ^= recv_ctx.LCT[j];
	//for(int i = 0; i < 0x10; ++i)
		//printf("%02x ", temp[i]);
	//printf("\n");
	*plen = ((int)temp[0] << 8) + (int)temp[1];
	//printf(">%d\n", *plen);
	if(*plen <= 0 || *plen > BUFSIZE) return false;
	blk_len = *plen + 2;
	if(blk_len & 0x0f)
		blk_len = (blk_len & 0xfffffff0) + 0x10;
	//printf("connection.c:176 blk_len = %d\n", blk_len);
	if((ret = recv_data(buffer + 0x10, blk_len - 0x10 + 20, 0)) <= 0){
		//printf("%d\n", ret);	
		return ret;
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
#ifdef DEBUG
	printf("recv %d\n", *plen);
	for(int i = 0; i < *plen; ++i)
		if(msg[i] > 32 && msg[i] < 127)
			printf("%5c", msg[i]);
		else
			printf("%#5x", (byte)msg[i]);
	printf("\n");
#endif
	return true;
}
