#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <fcntl.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "remote_control.h"
#include "communication.h"

extern int mode_of_sys;
extern int mode_of_work;
extern unsigned int host;
extern unsigned short port;
extern int client;
char message[BUFSIZE + 5];

bool get_file(){
    char file_path[BUFSIZE + 5] = {0};
    char message[BUFSIZE + 5] = {0};
    int file_path_len;
    int fd;
    int i, sum = 0;
    recv_msg(file_path, &file_path_len);
	file_path[file_path_len] = '\0';
	//printf("%s\n", file_path);
    if((fd = open(file_path, O_RDONLY)) < 0) return false;
	printf("here\n");
    while(1){
		i = read(fd, message, BUFSIZE);
		printf("%d\n", i);
		if(i <= 0) return false;
		send_msg(message, i);
		sum += i;
	}
	return true;
}

bool put_file(){
    int fd;
	int i, sum = 0;
	int file_path_len;
	char file_path[BUFSIZE + 5] = {0};
	recv_msg(file_path, &file_path_len);
	if(fd = creat(file_path, 0644) < 0) return false; //to be perfected
	while(1){	//to be perfected
		recv_msg(message, &i);	//to be perfected
		if(i <= 0) return false;
		if(write(fd, message, i) != i) return false; //exception handling;
		sum += i;
	}
}

void run_shell(){
    fd_set rd;
    int len;
    char message[BUFSIZE + 5] = {0};
    while( 1 )
    {
        FD_ZERO( &rd );
        FD_SET( client, &rd );
        FD_SET( 1, &rd );
        if(select(client + 1, &rd, NULL, NULL, NULL) < 0)
            return;
        if( FD_ISSET( client, &rd ) ){
            recv_msg( message, &len );
            write( 0, message, len );
        }
        if( FD_ISSET( 1, &rd ) )
        {
            len = read( 1, message, BUFSIZE );
            send_msg(message, len);
        }
    }
}

void service(){
    char command_msg;
	int msg_len;
    recv_msg(&command_msg, &msg_len);
	//printf("%d\n", command_msg);
    switch(command_msg){
    	case GET_FILE:get_file();break;
    	case PUT_FILE:put_file();break;
    	case RUN_SHELL:run_shell();break;
    	default:;
    }
}

void usage(){
	printf("\t-f\tforward connection,default mode.\n");
	printf("\t-r\treverse connection.\n");
	printf("\t-i\tip address.\n");
	printf("\t-p\tport.\n");
	printf("\t-h\tprint help message.");
	exit(0);
}

int main(int argc, char *argv[]){
    int ch;
    mode_of_sys = SERVER;
    while((ch = getopt(argc, argv, "frhi:p:")) != -1){
		switch(ch){
		    case 'f':
				mode_of_work = FORWARDCON;
				break;
			case 'r':
				mode_of_work = REVERSECON;
				break;
			case 'i':
				host = inet_addr(optarg);
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case 'h':
				usage();
				break;
			default:
				printf("Existence of unidentified parameters\n");
		}
	}
	if(init_connection() == FAILURE){
		printf("connect failure\n");
	}
    else{
        service();
    }
	return 0;
}
