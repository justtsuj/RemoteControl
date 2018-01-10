#include "remote_control.h"
#include "communication.h"


char command_line[BUFSIZE + 5];
char command[BUFSIZE + 5];
char opt_s[BUFSIZE + 5];
char opt_d[BUFSIZE + 5];

void parse_command(){
	int len = strlen(command_line);
	int i, j = 0;
	for(i = 0; i < len && command_line[i] != ' '; ++i, ++j){
		command[j] = command_line[i];
	}
	while(i < len && command_line[i] == ' ') ++i;	//filter space
	for(j = 0; i < len && command_line[i] != ' '; ++i, ++j){
		opt_d[j] = command_line[i];
	}
	while(i < len && command_line[i] == ' ') ++i;
	for(j = 0; i < len && command_line[i] != ' '; ++i, ++j){
		opt_s[j] = command_line[i];
	}
	//exception handling;
}

void get_file(char *dest_path, char *sour_path){	//point out the meaning of parameter
	char *tmp;
	int fd;
	int i, sum = 0;
	int len = strlen(sour_path);
	send_msg(sour_path, len);
	tmp = strrchr(sour_path, '/');
	if(tmp == NULL)
		tmp = sour_path;
	else
		++tmp;
	len = strlen(dest_path);
	/*if(dest_path[len - 1] != '/')
		dest_path[len] = '/';*/
	strcat(dest_path, sour_path);
	fd = creat(dest_path, 0644);
	if(fd < 0) return;	//to be perfected
	while(1){	//to be perfected
		recv_msg(message, &i);	//to be perfected
		if(i <= 0) return;
		if(write(fd, message, i) != i) return; //exception handling;
		sum += i;
	}
}

void put_file(char *dest_path, char *sour_path){
	int sum = 0;
	int fd;
	char *tmp = strrchr(sour_path, '/');
	if(tmp == NULL)
		tmp = sour_path;
	else
		++tmp;
	strcat(dest_path, sour_path);
	send_msg(dest_path, strlen(dest_path));
	if(fd = open(sour_path, O_RDONLY) < 0) return;
	while(1){
		int i = read(fd, message, BUFSIZE);
		if(i <= 0) return;
		send_msg(message, i);
		sum += i;
	}
}

void run_shell(char *shell_command){
	fd_set rd;
	int len;
	send_msg(shell_command, strlen(shell_command));
	while(1){
		FD_ZERO(&rd);
		FD_SET(0, rd);
		FD_SET(client, &rd);
		select(client + 1, &rd, NULL, NULL, NULL)
		if(FD_ISSET(client, &rd)){
			recv_msg(message, &len);
			write(1, message, len);
		}
		if(FD_ISSET(0, &rd)){
			len = read(0, message, BUFSIZE);
			send_msg(message, len);
		}
	}
}

void exec_command(){
	char command_msg;
	parse_command();
	if(strlen(command) > 0){
		if(strcmp(command, "shell") == 0){
			command_msg = RUN_SHELL;
			send_msg(&command_msg, 1);
			run_shell(opt_d);	//exception handling
		}
		else if(strcmp(command, "get") == 0){
			command_msg = GET_FILE;
			send_msg(&command_msg, 1);
			//download a file from opt_s to opt_d;
			get_file(opt_d, opt_s);	//exception handling
		}
		else if(strcmp(command, "put") == 0){
			command_msg = PUT_FILE;
			send_msg(&command_msg, 1);
			//upload a file from opt_s to opt_d;
			put_file(opt_d, opt_s);	//exception handling
		}
		else{
			printf("unkonwn command");
		}
	}
}

int main(int argc, char *argv[]){
	int ch;
	char command[100];
    service_object = CLIENT;
	while((ch = getopt(argc, argv, "rh:p:")) != -1){
		switch(ch){
			case 'r':
				mode_of_work = REVERSECON;
				break;
			case 'h':
				host = atoi(optarg);
				break;
			case 'p':
				port = atoi(optarg);
				break;
			default:
				printf("usage\n");
		}
	}

	if(test_connection() == FAILURE){
		printf("connect failure\n");
	}else{
		fgets(command_line, BUFSIZE - 1, stdin);
		exec_command();
	}
	return 0;
}
