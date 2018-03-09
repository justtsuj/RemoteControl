#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "remote_control.h"
#include "communication.h"

char command_line[BUFSIZE + 5];
char command[BUFSIZE + 5];
char opt_first[BUFSIZE + 5];
char opt_second[BUFSIZE + 5];
char message[BUFSIZE + 5];
extern int mode_of_sys;
extern int mode_of_work;
extern unsigned int host;
extern unsigned short port;
extern int client;



bool parse_command(){
	int len = strlen(command_line);
	int i = 0, j;
	memset(command, 0, sizeof(command));
	memset(opt_first, 0, sizeof(opt_first));
	memset(opt_second, 0, sizeof(opt_second));
	--len;	//filter '\n'
	for(j = 0; i < len && command_line[i] != ' '; ++i, ++j){
		command[j] = command_line[i];
	}
	if(strlen(command) == 0)
		return false;
	while(i < len && command_line[i] == ' ') ++i;	//filter space
	for(j = 0; i < len && command_line[i] != ' '; ++i, ++j){
		opt_first[j] = command_line[i];
	}
	while(i < len && command_line[i] == ' ') ++i;
	for(j = 0; i < len && command_line[i] != ' '; ++i, ++j){
		opt_second[j] = command_line[i];
	}
    return true;
	//exception handling;
}

bool get_file(char *dest_path, char *sour_path){	//point out the meaning of parameter
	char *tmp;
	int fd;
	int i, j, sum = 0;
	//printf("%s\n", sour_path);
	send_msg(sour_path, strlen(sour_path));
	tmp = strrchr(sour_path, '/');
	if(tmp == NULL)
		return false;
	++tmp;
	/*len = strlen(dest_path);
	if(dest_path[len - 1] != '/')
		dest_path[len] = '/';*/
	strcat(dest_path, tmp);
	//printf("%s\n", dest_path);
	fd = creat(dest_path, 0644);    //to be perfected
	if(fd < 0) return false;	//to be perfected
	while(1){	//to be perfected
		recv_msg(message, &i);	//to be perfected
		printf("%d\n", i);
		if(i <= 0) return false;
		j = write(fd, message, i);
		if(j != i) return false; //exception handling;
		printf("%d\n", j);
		sum += i;
	}
	printf("here\n");
	return true;
}

bool put_file(char *dest_path, char *sour_path){
	int sum = 0;
	int fd;
	char *tmp = strrchr(sour_path, '/');
	if(tmp == NULL)
		return false;
    ++tmp;
	strcat(dest_path, tmp);
	send_msg(dest_path, strlen(dest_path));
	if(fd = open(sour_path, O_RDONLY) < 0) return false;
	while(1){
		int i = read(fd, message, BUFSIZE);
		if(i <= 0) return false;
		send_msg(message, i);
		sum += i;
	}
}

bool run_shell(char *shell_command){
	fd_set rd;
	int len;
	send_msg(shell_command, strlen(shell_command));
	while(1){
		FD_ZERO(&rd);
		FD_SET(0, &rd);
		FD_SET(client, &rd);
		select(client + 1, &rd, NULL, NULL, NULL);
		if(FD_ISSET(client, &rd)){
			recv_msg(message, &len);
			write(1, message, len);
		}
		if(FD_ISSET(0, &rd)){
			len = read(0, message, BUFSIZE);
			send_msg(message, len);
		}
	}
	//exception handling;
	return true;
}

bool exec_command(){
	char command_msg;
	parse_command();
	if(strcmp(command, "shell") == 0){
    	if(strlen(opt_first) == 0)
        	strcpy(opt_first, "exec bash --login");
		command_msg = RUN_SHELL;
		send_msg(&command_msg, 1);
		run_shell(opt_first);	//exception handling
	}
	else if(strcmp(command, "get") == 0){
        if(strlen(opt_first) == 0) return false;
		command_msg = GET_FILE;
		send_msg(&command_msg, 1);
		//download a file from opt_s to opt_d;
		if(strlen(opt_second))
            get_file(opt_first, opt_second);	//exception handling
        else{
			strcat(opt_second, "./");
            get_file(opt_second, opt_first);
		}
	}
	else if(strcmp(command, "put") == 0){
        if(strlen(opt_first) == 0) return false;
		command_msg = PUT_FILE;
		send_msg(&command_msg, 1);
		//upload a file from opt_s to opt_d;
		if(strlen(opt_second))
            put_file(opt_first, opt_second);	//exception handling
        else{
			strcat(opt_second, "./");
            put_file(opt_second, opt_first);
		}
	}
	else
		printf("unkonwn command");
	return true;
}

int main(int argc, char *argv[]){
	int ch;
	mode_of_sys = CLIENT;
	while((ch = getopt(argc, argv, "frh:p:")) != -1){
		switch(ch){
		    case 'f':
				mode_of_work = FORWARDCON;
				break;
			case 'r':
				mode_of_work = REVERSECON;
				break;
			case 'h':
				host = inet_addr(optarg);
				break;
			case 'p':
				port = atoi(optarg);
				break;
			default:
				printf("usage\n");
		}
	}

	if(init_connection() == FAILURE){
		printf("connect failure\n");
	}else{
		fgets(command_line, BUFSIZE - 1, stdin);
		exec_command();
	}
	return 0;
}
