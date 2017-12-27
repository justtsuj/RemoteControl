unsigned char buffer[BUFSIZE + 16 + 20];
unsigned char message[BUFSIZE + 5];

void send_all(int sockfd, int len, int flags){
	int send_msg_len;
	int cur = 0;
	while(cur < len){
		send_msg_len = send(sockfd, buffer + cur, len - cur, flags);
		if(send_msg_len < 0)
			return;
		cur += send_msg_len;
	}
}

void send_msg(int sockfd, char *msg, int len){
	int blk_len;
	int i, j;
	if(len <=0 || len > BUFSIZE)
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
	send_all(sockfd, buffer, len + 2, 0);
}

void recv_all(int sockfd, int offset, int len, int flags){
	int recv_msg_len;
	int cur = 0;
	while(cur < len){
		recv_msg_len = recv(sockfd, buffer + offset + cur, len - cur, flags);
		cur += recv_msg_len;
	}
}

void recv_msg(int sockfd, char *msg, int *plen){
	recv_all(sockfd, buffer, 2, 0);
	*plen = ((int)buffer[0] << 8) + (int)buffer[1];
	if(*plen <= 0 || *plen > BUDSIZE) return ;
	recv_all(sockfd, buffer + 2, *plen, 0);
	memcpy(msg, buffer + 2, *plen);
}

void get_file(int sockfd, char *dest_path, char *sour_path){	//point out the meaning of parameter
	char *tmp;
	int fd;
	int i, sum = 0;
	int len = strlen(sour_path);
	send_msg(sockfd, sour_path, len);
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
		recv_msg(sockfd, message, &i);	//to be perfected
		if(i <= 0) return;
		if(write(fd, message, i) != i) return; //exception handling;
		sum += i;
	}
}

void put_file(int sockfd, char *dest_path, char *sour_path){
	int sum = 0;
	char *tmp = strrchr(sour_path, '/');
	if(tmp == NULL)
		tmp = sour_path;
	else
		++tmp;
	strcat(dest_path, sour_path);
	send_msg(sockfd, dest_path, strlen(dest_path));
	if(open(sour_path, O_RDONLY) < 0) return;
	while(1){
		int i = read(fd, message, BUFSIZE);
		if(i <= 0) return;
		send_msg(sockfd, message, i);
		sum += i;
	}
}

void run_shell(int sockfd, char *shell_command){
	fd_set rd;
	int len;
	send_msg(sockfd, shell_command, strlen(shell_command));
	while(1){
		FD_ZERO(&rd);
		FD_SET(0, rd);
		FD_SET(sockfd, &rd);
		select(sockfd + 1, &rd, NULL, NULL, NULL)
		if(FD_ISSET(sockfd, &rd)){
			recv_msg(sockfd, message, &len);
			write(1, message, len);
		}
		if(FD_ISSET(0, &rd)){
			len = read(0, message, BUFSIZE);
			send_msg(sockfd, message, len);
		}
	}
}
