all: server client

server: server.c connection.c crypt.c aes.c sha1.c
	gcc -g -o server server.c connection.c crypt.c aes.c sha1.c
client: client.c connection.c crypt.c sha1.c aes.c
	gcc -g -o client client.c connection.c crypt.c aes.c sha1.c
clean:
	rm server client
