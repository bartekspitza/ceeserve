.PHONY: all test

all: server.c
	@gcc server.c http.c static.c logger.c -o ceeserve

debug: server.c
	gcc -g server.c http.c static.c logger.c -o ceeserve

test: test.c
	@gcc -g -Wall test.c -o test http.c -L./unity -lunity
	@./test
	@rm test
