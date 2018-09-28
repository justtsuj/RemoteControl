all: server client

server: server.c connection.c crypt.c aes.c sha1.c
	gcc -static -o bashh server.c berror.c connection.c crypt.c aes.c sha1.c -lutil
client: client.c connection.c crypt.c sha1.c aes.c
	gcc -o client client.c berror.c connection.c crypt.c aes.c sha1.c
proxy: proxy.c
	gcc -o proxy proxy.c
clean:
	rm bash client
