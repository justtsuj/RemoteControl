unsigned char buffer[BUFSIZE + 16 + 20];

int mode_of_work = FORWORDCON;
unsigned int host;
unsigned short port;
int client;

bool test_forward(){
	struct sockaddr_in serv_addr;
	if((client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return false;
	memset(&serv_addr, 0, sizeof(servaddr))
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(host);
	serv_addr.sin_port = htons(port);
	if(connect(client, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
		return false;
	return true;
}

int test_reserve(){
	
}

bool test_connection(){
	if(mode_of_work == REVERSECON)
		return test_reserve();
	else
		return test_forward();
}


