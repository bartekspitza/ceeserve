.PHONY: all test
CFLAGS= -g -Wall

all: server.c
	@gcc server.c http.c static.c logger.c -o out/server

debug: server.c
	gcc -g server.c http.c static.c logger.c -o out/server

test: test.c
	@gcc $(CFLAGS) test.c -o test http.c -L./unity -lunity
	@./test
	@rm test
