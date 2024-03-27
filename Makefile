all: server.c
	@gcc server.c http.c -o out/server
