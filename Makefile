all: server.c
	@gcc server.c http.c -o out/server

test: test.c
	@gcc test.c -o test http.c -L./unity -lunity
	@./test
	@rm test