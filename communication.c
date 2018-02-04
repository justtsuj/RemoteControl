#include "communication.h"

unsigned char buffer[BUFSIZE + 5] = {0};    //send_data,recv_data buffer

int mode_of_work = FORWORDCON;
unsigned int host;
unsigned short port;
int client;


//whether to close the socket when a connection fails
bool create_client_socket(){
    struct sockaddr_in addr;
    if((client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return false;
    memset(&serv_addr, 0, sizeof(servaddr))
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(host);
    addr.sin_port = htons(port);
    if(connect(client, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        return false;
    return true;
}

//socket setting
bool create_server_socket(){
    int server;
    struct sockaddr_in serv_addr, clie_addr;
    int clie_addr_len;
    if(server = socket(AF_INET, SOCK_STREAM, 0) < 0)
        return false;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(server, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        return false;
    if(listen(server, 5 ) < 0)
        return false;
    //whether the output shoule be added
    if(client = accept(client, (struct sockaddr *)&clie_addr, &clie_addr_len) < 0)
        reurn false;
    return true;
}

bool init_connection(){
    if(system ^ mode_of_work)
        return create_client_socket();
    else
        return create_server_socket();
}

//send a complete data
//include error handing
bool send_data(int len, int flags){
	int send_data_len;
	int cur = 0;
	while(cur < len){
		send_data_len = send(client, buffer + cur, len - cur, flags);
		if(send_msg_len < 0)
			return false;
		cur += send_data_len;
	}
	return true;
}

bool send_msg(char *msg, int len){
	int blk_len;
	int i, j;
	if(len <= 0 || len > 0xffff || len > BUFSIZE - 2)
		return;
	buffer[0] = (len >> 8) & 0xff;
	buffer[1] = len & 0xff;
	memcpy(buffer + 2, msg, len);	//header file string.h
	/*blk_len = 2 + length;
	if((blk_len & 0xf) != 0)
		blk_len += 16 - (blk_len & 0xf);
	for(i = 0; i < blk_len; i += 16){
		for(j = 0; j < 16; ++j){
			buffer[i + j] ^= send_ctx.LCT[j];
		}
		aes_encrypt(&send_ctx.SK, &buffer[i]);
		memcpy(send_ctx.LCT, &buffer[i], 16);
	}*/
	return send_data(len + 2, 0);
}

bool recv_data(int len, int flags){
	int recv_msg_len;
	int cur = 0;
	while(cur < len){
		recv_msg_len = recv(client, buffer + cur, len - cur, flags);
		if(recv_msg_len < 0)
			return false;
		cur += recv_msg_len;
	}
	return true;
}

bool recv_msg(char *msg, int *plen){
	if(recv_data(2, 0) == FAILURE) return false;
	*plen = ((int)buffer[0] << 8) + (int)buffer[1];
	if(*plen <= 0 || *plen > BUFSIZE) return false;
	if(recv_all(*plen, 0) == FAILURE) return false;
	memcpy(msg, buffer, *plen);
}

