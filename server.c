#include "remote_control.h"

bool get_file(){
    char file_path[BUFSIZE + 5] = {0};
    char message[BUFSIZE + 5] = {0};
    int file_path_len;
    int fd;
    int i, sum = 0;
    recv_msg(file_path, &file_path_len);
    if(fd = open(sour_path, O_RDONLY) < 0) return false;
    while(1){
		int i = read(fd, message, BUFSIZE);
		if(i <= 0) return false;
		send_msg(message, i);
		sum += i;
	}
}

bool put_file(){
    int fd;
	int i, sum = 0;
	int file_path_len;
	char file_path[BUFSIZE + 5] = {0};
	recv_msg(file_path, &file_path_len);
	if(fd = creat(dest_path, 0644) < 0) return false; //to be perfected
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
            send_msg( client, message, len );
        }
    }
}

void service(){
    char command_msg;
    recv_msg(&command_msg, 1);
    switch(command_msg){
    case GET_FILE:get_file();break;
    case PUT_FILE:put_file();break;
    case RUN_SHELL:run_shell();break;
    default:
    }
}

int main(int argc, char *argv[]){
    int ch;
    system = SERVER;
    while((ch = getopt(argc, argv, "frh:p:")) != -1){
		switch(ch){
		    case 'f':
				mode_of_work = FORWARDCON;
				break;
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
	if(init_connection() == FAILURE){
		printf("connect failure\n");
	}
    else{
        service();
    }
	return 0;
}
