#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void logger(const char* tag, const char* message) {
   time_t now;
   time(&now);
   struct tm *timeinfo = localtime(&now);
   char timestr[50];
   strftime(timestr, 50, "%F %X", timeinfo);
   printf("%s [%s]: %s\n", timestr, tag, message);
}