all: http_client http_server
http_client: client.c
	gcc -o http_client client.c
http_server: server.c
	gcc -o http_server server.c -lpthread
clean:
	rm -f http_client http_server
