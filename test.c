#include "./unity/unity.h"
#include "http.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


void http_request_parse_http11_supported() {
    char *request = "GET / HTTP/1.1\n\
    Host: localhost:8080\r\n";

    http_request_t req = {0};
    http_request_parse(request, &req);
}

void http_request_parse_http11_protocol_failure() {
    char *request = "GET /";

    http_request_t req = {0};
    int res = http_request_parse(request, &req);
    TEST_ASSERT_EQUAL_INT16(-1, res);
}

void http_request_parse_http11_request_line() {
    char *request = "GET / HTTP/1.1\n\
    Host: localhost:8080\r\n";

    http_request_t req = {0};
    http_request_parse(request, &req);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("GET", req.method, 3);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("HTTP/1.1", req.version, 8);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("/", req.path, 1);
}
void http_request_parse_http11_headers() {
    char request[] = "GET / HTTP/1.1\n\
Host: localhost:8080\r\n\
Header2:value2\n";

    http_request_t req = {0};
    http_request_parse(request, &req);
    TEST_ASSERT_EQUAL_INT16(2, req.header_count);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("Host", req.headers[0].key, 4);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("localhost:8080", req.headers[0].value, strlen("localhost:8080"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("Header2", req.headers[1].key, strlen("Header2"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("value2", req.headers[1].value, strlen("value2"));
}

int main(int argc, char *argv[]) {


    UNITY_BEGIN();

    RUN_TEST(http_request_parse_http11_supported);
    RUN_TEST(http_request_parse_http11_protocol_failure);
    RUN_TEST(http_request_parse_http11_request_line);
    RUN_TEST(http_request_parse_http11_headers);

    UNITY_END();


    return 0;
}

// Unity functions
void setUp(void) {}
void tearDown(void) {}
