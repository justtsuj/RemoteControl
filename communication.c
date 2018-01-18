#include "remote_control.h"
#include "communication.h"

unsigned char buffer[BUFSIZE + 5] = {0};    //send_data,recv_data buffer

int service_object;
int mode_of_work = FORWORDCON;
unsigned int host;
unsigned short port;
int client;

void create_client_socket(){

}

void create_server_socket(){

}

bool test_forward(){
    if(service_object == CLIENT){
        struct sockaddr_in serv_addr;
        if((client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            return false;
        memset(&serv_addr, 0, sizeof(servaddr))
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(host);
        serv_addr.sin_port = htons(port);
        if(connect(client, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
            return false;
        return true;
    }
    else if(service_object == SERVER){
        create_server_socket()
    }
	else{

	}
}

int test_reserve(){
    if(service_object == CLIENT){
        struct sockaddr_in serv_addr;
        if((client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            return false;
        memset(&serv_addr, 0, sizeof(servaddr))
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(host);
        serv_addr.sin_port = htons(port);
        if(connect(client, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
            return false;
        return true;
    }
    else if(service_object == SERVER){
        create_server_socket()
    }
	else{

	}
}

bool test_connection(){
	if(mode_of_work == REVERSECON)
		return test_reserve();
	else
		return test_forward();
}

//send a complete data
//include error handing
void send_data(int len, int flags){
	int send_msg_len;
	int cur = 0;
	while(cur < len){
		send_msg_len = send(client, buffer + cur, len - cur, flags);
		if(send_msg_len < 0)
			return;
		cur += send_msg_len;
	}
}

void send_msg(char *msg, int len){
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
	send_all(len + 2, 0);
}

void recv_data(int len, int flags){
	int recv_msg_len;
	int cur = 0;
	while(cur < len){
		recv_msg_len = recv(client, buffer + cur, len - cur, flags);
		cur += recv_msg_len;
	}
}

void recv_msg(char *msg, int *plen){
	recv_all(2, 0);
	*plen = ((int)buffer[0] << 8) + (int)buffer[1];
	if(*plen <= 0 || *plen > BUFSIZE) return;
	recv_all(*plen, 0);
	memcpy(msg, buffer, *plen);
}

