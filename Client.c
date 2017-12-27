#include "RemoteContral.h"
int mode_of_work = FORWORDCON;
unsigned int host;
unsigned short port;
char command_line[BUFSIZE];
char command[BUFSIZE];
char opt_s[BUFSIZE];
char opt_d[BUFSIZE];

void send_msg(int sockfd, char *msg, int len);
void get_file(int sockfd, char *dest_path, char *sour_path);
void put_file(int sockfd, char *dest_path, char *sour_path);
void run_shell(int sockfd, char *shell_command);

int test_forward(){
	bool res = true;
	int client;
	struct sockaddr_in serv_addr;
	if((client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;
	memset(&serv_addr, 0, sizeof(servaddr))
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(host);
	serv_addr.sin_port = htons(port);
	if(connect(client, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
		return -1;
	return client;
}

int test_reserve(){
	
}

int test_connection(){
	if(mode_of_work == REVERSECON)
		return test_reserve();
	else
		return test_forward();
}

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

int create_con(){
	return test_connection();
}

bool exec_command(){
	bool res = true;
	int sock;
	char command_msg;
	parse_command();
	if(strlen(command) > 0){
		if((sock = create_con()) < 0)
			return false;
		if(strcmp(command, "shell") == 0){
			command_msg = RUN_SHELL;
			send_msg(sock, &command_msg, 1)
			run_shell(sock, opt_d);	//exception handling
		}
		else if(strcmp(command, "get") == 0){
			command_msg = GET_FILE;
			send_msg(sock, &command_msg, 1)
			//download a file from opt_s to opt_d;
			get_file(sock, opt_d, opt_s);	//exception handling
		}
		else if(strcmp(command, "put") == 0){
			command_msg = PUT_FILE;
			send_msg(sock, &command_msg, 1)
			//upload a file from opt_s to opt_d;
			put_file(sock, opt_d, opt_s);	//exception handling
		}
		else{
			printf("unkonwn command");
			res = false;
		}
		close(sock);
	}
	return res;
}

int main(int argc, char *argv[]){
	int mode_of_work = FORWORDCON;
	int ch;
	bool is_unreachable, is_execed;
	char command[100];

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

	is_unreachable = test_connection();
	if(is_unreachable){
		printf("connect failure\n");
	}else{
		fgets(command_line, BUFSIZE - 1, stdin);
		printf("%s\n", exec_command() ? "Done" : "Faliure");
	}
	return 0;
}
