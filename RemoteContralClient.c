#include "RemoteContral.h"

int mode_of_work = FORWORDCON;
unsigned int host;
unsigned short port;
char command_line[100];
char command[100];
char opt_s[100];
char opt_d[100];

bool test_forward(){
	int client;
	struct sockaddr_in serv_addr;
	if((client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return false;
	memset(&serv_addr, 0, sizeof(servaddr))
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(host);
	serv_addr.sin_port = htons(port);
	if(connect(client, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		close(client);
		return false;
	}
	close(client);
	return true;
}

bool test_connection(){
	if(mode_of_work == REVERSECON){
		return test_reserve();
	}
	else{
		return test_forward();
	}
}

void parse_command(){
	
}
bool exec_command(){
	int sock;
	parse_command();
	if(strlen(command) > 0){
		if((sock = create_con()) < 0)
			return false;
		send_msg(sock, command, );	//to be perfected
		if(strcmp(command, "shell") == 0){
			runshell(sock, opt_d);
		}
		else if(strcmp(command, "get") == 0){
			//download a file from opt_s to opt_d;
			get_file(sock, opt_d, opt_s);
		}
		else if(strcmp(command, "put") == 0){
			//upload a file from opt_s to opt_d;
			put_file(sock, opt_d, opt_s);
		}
		else{
			close(sock);
			printf("unkonwn command");
			return false;
		}
		close(sock);
	}
	return true;
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
		fgets(command_line, 99, stdin);
		printf("%s\n", exec_command() ? "Done" : "Faliure");
	}
	return 0;
}
