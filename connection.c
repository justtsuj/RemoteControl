#include "basic.h"
#include "connection.h"
#include "crypt.h"
#include "sha1.h"

byte buffer[BUFSIZE + 20] = {0};    //send_data,recv_data buffer
int mode_of_sys;
int mode_of_work = FORWARDCON;
unsigned int host;
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
	if((server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
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
	if((client = accept(server, (struct sockaddr *)&clie_addr, &clie_addr_len)) < 0){
		close(server);
		return false;
	}
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
	}
	return true;
}

bool send_msg(char *msg, int len){
	int blk_len;
	int i, j;
	struct sha1_context sha1_ctx;
	if(len <= 0 || len > BUFSIZE)	//redefine length
		return false;
	buffer[0] = (len >> 8) & 0xff;
	buffer[1] = len & 0xff;
	memcpy(buffer + 2, msg, len);	//header file string.h
	encrypt(buffer, len + 2);
	return send_data(buffer, len + 2 + 20, 0);
}

bool recv_data(byte *loc, int len, int flags){
	int recv_msg_len;
	int cur = 0;
	while(cur < len){
		recv_msg_len = recv(client, loc + cur, len - cur, flags);
		if(recv_msg_len <= 0)
			return false;
		cur += recv_msg_len;
	}
	return true;
}

//set max message length
bool recv_msg(char *msg, int *plen){
	int blk_len;
	int j;
	byte temp[0x10];
	if(recv_data(buffer, 0x10, 0) == FAILURE) return false;
	memcpy(temp, buffer, 0x10);
	aes_decrypt(&recv_ctx.SK, temp);
	for(j = 0; j < 0x10; ++j)
		temp[j] ^= recv_ctx.LCT[j];
	*plen = ((int)temp[0] << 8) + (int)temp[1];
	//printf(">%d\n", *plen);
	if(*plen <= 0 || *plen > BUFSIZE) return false;
	blk_len = *plen + 2;
	if(blk_len & 0x10)
		blk_len = (blk_len & 0xfffffff0) + 0x10;
	if(recv_data(buffer + 0x10, blk_len - 0x10 + 20, 0) == FAILURE) return false;
	if(decrypt(buffer, blk_len + 20) == FAILURE) return false;
	memcpy(msg, buffer + 2, *plen);
	return true;
}
