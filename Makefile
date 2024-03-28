.PHONY: all test
CFLAGS= -g -Wall

all: server.c
	@gcc server.c http.c static.c -o out/server

test: test.c
	@gcc $(CFLAGS) test.c -o test http.c -L./unity -lunity
	@./test
	@rm test
