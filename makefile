server: server.c string.c
	gcc -o server server.c string.c -pthread -lcrypto
client: client.c string.c aes.c
	gcc -o client client.c string.c aes.c -pthread -lcrypto -g
aes: aes.c
	gcc -o aes aes.c -lcrypto