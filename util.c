#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <util.h>

#define streql(a,b) (!strcmp((a),(b)))

void print_help() {
    printf(
        "pa-cli [device] [action] [value]\n"
        "Examples:\n"
        "    pa-cli [sink | source] mute\n"
        "    pa-cli [sink | source] list\n"
        "    pa-cli [sink | source] adjust-volume 10\n"
        "    pa-cli [sink | source] adjust-volume -10\n"
        "    pa-cli [sink | source] set-default <device_name>\n"
        "    pa-cli [sink | source] is-muted\n"
        "    pa-cli [sink | source] get-volume\n"
    );
}

void warnf(const char *format, ...) {
    printf("\033[0;33m");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    printf("\033[0m");
}


void errorf(const char *format, ...) {
    fprintf(stderr, "\033[0;31m");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\033[0m");
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

const char *strnull(const char *str) {
    return str ? str : "(null)";
}
