all: server client

server: server.c communication.c
	gcc -g -o server server.c communication.c -lutil

client: client.c communication.c
	gcc -g -o client client.c communication.c
