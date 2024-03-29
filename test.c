#include "./unity/unity.h"
#include "http.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void test_parse_headers_request_line_failure() {
    char *request = "GET /";

    HttpRequest req = {0};
    int res = parse_headers(request, &req);
    TEST_ASSERT_EQUAL_INT16(-1, res);
}

void test_parse_headers_request_line() {
    char *request = "GET / HTTP/1.1\r\n\
    Host: localhost:8080\r\n\r\n";

    HttpRequest req = {0};
    parse_headers(request, &req);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("GET", req.method, 3);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("HTTP/1.1", req.version, 8);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("/", req.path, 1);
}

void test_parse_headers_headers_failure_no_trailing_newline() {
    char request[] = "\
GET / HTTP/1.1\r\n\
Host: localhost:8080";

    HttpRequest req = {0};
    int res = parse_headers(request, &req);
    TEST_ASSERT_EQUAL_INT16(-1, res);
}

void test_parse_headers_headers() {
    char request[] = "\
GET / HTTP/1.1\r\n\
Host: localhost:8080\r\n\r\n";

    HttpRequest req = {0};
    parse_headers(request, &req);
    TEST_ASSERT_EQUAL_INT16(1, req.header_count);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("Host", req.headers[0].key, 4);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("localhost:8080", req.headers[0].value, strlen("localhost:8080"));
}

void test_parse_headers_headers_withcr() {
    char request[] = "\
GET / HTTP/1.1\r\n\
Host: localhost:8080\r\n\r\n";

    HttpRequest req = {0};
    parse_headers(request, &req);
    TEST_ASSERT_EQUAL_INT16(1, req.header_count);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("Host", req.headers[0].key, 4);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("localhost:8080", req.headers[0].value, strlen("localhost:8080"));
}

void test_parse_headers_headers_multiple() {
    char request[] = "\
GET / HTTP/1.1\r\n\
Host: localhost:8080\r\n\
Header2:value2\r\n\r\n";

    HttpRequest req = {0};
    parse_headers(request, &req);
    TEST_ASSERT_EQUAL_INT16(2, req.header_count);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("Host", req.headers[0].key, 4);
    TEST_ASSERT_EQUAL_CHAR_ARRAY("localhost:8080", req.headers[0].value, strlen("localhost:8080"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("Header2", req.headers[1].key, strlen("Header2"));
    TEST_ASSERT_EQUAL_CHAR_ARRAY("value2", req.headers[1].value, strlen("value2"));
}

void test_get_header() {
    char* key = "Content-Type";
    char* value = "application/json";
    HttpHeader h1 = {
        .key = key,
        .value = value,
    };
    HttpHeader headers[] = {h1};
    HttpRequest req = {
        .headers = headers,
        .header_count = 1,
    };

    HttpHeader* fetch = get_header(req, key);
    TEST_ASSERT_EQUAL_CHAR_ARRAY(key, fetch->key, strlen(key));
    TEST_ASSERT_EQUAL_CHAR_ARRAY(value, fetch->value, strlen(value));
}

void test_get_header_not_found_returns_null() {
    char* key = "Content-Type";
    char* value = "application/json";
    HttpHeader h1 = {
        .key = key,
        .value = value,
    };
    HttpHeader headers[] = {h1};
    HttpRequest req = {
        .headers = headers,
        .header_count = 1,
    };

    HttpHeader* fetch = get_header(req, "somekey");
    TEST_ASSERT_NULL(fetch);
}

void test_http_response_tostr_response_line() {
    HttpResponse resp = {
        .version = "HTTP/1.1",
        .status_code = 400,
        .status_desc = "Bad Request",
        .headers = NULL,
        .header_count = 0,
        .body = NULL,
    };

    char *resptext = resptostr(resp);

    char correct[] = "HTTP/1.1 400 Bad Request\r\n\r\n\0";
    TEST_ASSERT_EQUAL_CHAR_ARRAY(correct, resptext, strlen(correct));
}

void test_http_response_tostr_headers() {
    HttpHeader h1 = {
        .key = "server",
        .value = "ceeserve",
    };
    HttpHeader headers[] = {h1};
    HttpResponse resp = {
        .version = "HTTP/1.1",
        .status_code = 400,
        .status_desc = "Bad Request",
        .headers = headers,
        .header_count = 1,
        .body = NULL,
    };

    char *resptext = resptostr(resp);

    char correct[] = "HTTP/1.1 400 Bad Request\r\nserver: ceeserve\r\n\r\n";
    TEST_ASSERT_EQUAL_CHAR_ARRAY(correct, resptext, strlen(correct));
}

int main(int argc, char *argv[]) {
    UNITY_BEGIN();

    RUN_TEST(test_parse_headers_request_line);
    RUN_TEST(test_parse_headers_request_line_failure);
    RUN_TEST(test_parse_headers_headers_failure_no_trailing_newline);
    RUN_TEST(test_parse_headers_headers);
    RUN_TEST(test_parse_headers_headers_withcr);
    RUN_TEST(test_parse_headers_headers_multiple);
    RUN_TEST(test_get_header);
    RUN_TEST(test_get_header_not_found_returns_null);

    RUN_TEST(test_http_response_tostr_response_line);
    RUN_TEST(test_http_response_tostr_headers);

    UNITY_END();
    return 0;
}

// Unity functions
void setUp(void) {}
void tearDown(void) {}
