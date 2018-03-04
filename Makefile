all: server client

server: server.c communication.c
	gcc -o server server.c communication.c

client: client.c communication.c
	gcc -o client client.c communication.c
