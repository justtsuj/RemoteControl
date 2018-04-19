#include "basic.h"

void handle_error(int ret){
	switch(ret){
		case 0:
			break;
		case 1:
			fprintf(stderr, "[-]: Host parameter illegality\n"); break;
		case 2:
			fprintf(stderr, "[-]: Port parameter illegality\n"); break;
		case 3:
			perror("[-]: socket"); break;
		case 4:
			perror("[-]: connect"); break;
		case 5:
			perror("[-]: setsockopt"); break;
		case 6:
			perror("[-]: bind"); break;
		case 7:
			perror("[-]: listen"); break;
		case 8:
			fprintf(stderr, "[-]: Challenge authentication failure\n"); break;
		case 9:
			fprintf(stderr, "[-]: Parse command failure\n"); break;
		case 10:
			fprintf(stderr, "[-]: Uploading or downloading files requires at least one parameter\n"); break;
		case 11:
			fprintf(stderr, "[-]: Source file path illegality\n"); break;
		case 12:
			fprintf(stderr, "[-]: Destination file path illegality\n"); break;
		case 13:
			fprintf(stderr, "[-]: Source or destination file path too long\n"); break;
		case 14:
			perror("[-]: creat"); break;
		case 15:
			fprintf(stderr, "[-]: unkonwn command\n"); break;
		case 16:
			perror("[-]: creat"); break;
		case 17:
			perror("[-]: accept"); break;
		case 18:
			fprintf(stderr, "[-]: Receive command message fail\n"); break;
		case 19:
			perror("[-]: open"); break;
		case 20:
			fprintf(stderr, "[-]: Receive data fail\n"); break;
		case 21:
			fprintf(stderr, "[-]: Receive data too long\n"); break;
		case 22:
			fprintf(stderr, "[-]: Send data fail\n"); break;
		case 23:
			fprintf(stderr, "[-]: Send data too long\n"); break;
		default:;
	}
}
