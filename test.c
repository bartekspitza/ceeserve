#include "./unity/unity.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


void httpRequest() {
    TEST_ASSERT_EQUAL_CHAR('c','c');
}

int main(int argc, char *argv[]) {


    UNITY_BEGIN();
    RUN_TEST(httpRequest);
    UNITY_END();


    return 0;
}

// Unity functions
void setUp(void) {}
void tearDown(void) {}
