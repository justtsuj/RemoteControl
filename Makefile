all: server client

server: server.c connection.c crypt.c aes.c sha1.c
	gcc -g -o server server.c berror.c connection.c crypt.c aes.c sha1.c -lutil
client: client.c connection.c crypt.c sha1.c aes.c
	gcc -g -o client client.c berror.c connection.c crypt.c aes.c sha1.c
proxy: proxy.c
	gcc -g -o proxy proxy.c
clean:
	rm server client proxy
