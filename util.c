#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <util.h>

#define streql(a,b) (!strcmp((a),(b)))

void print_help() {
    printf("pa-cli [device] [action] [value]\n");
    printf("Examples:\n");
    printf("\tpa-cli sink mute\n");
    printf("\tpa-cli sink list\n");
    printf("\tpa-cli sink adjust-volume 10\n");
    printf("\tpa-cli sink adjust-volume -10\n");
    printf("\n");
    printf("\tpa-cli source mute\n");
    printf("\tpa-cli source list\n");
    printf("\tpa-cli source adjust-volume 10\n");
    printf("\tpa-cli source adjust-volume -10\n");
}

void errorf(const char *format, ...) {
    printf("\033[0;31m");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    printf("\033[0m");
}

char* strcopy(const char *src) {
    int len = strlen(src) + 1;
    char *dest = malloc(sizeof(char) * len);
    strcpy(dest, src);

    return dest;
}

int clamp(int value, int min, int max) {
    return (value > max) ? max : ((value < min ? min : value));
}
