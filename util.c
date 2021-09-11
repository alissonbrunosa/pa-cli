#include <stdio.h>
#include <util.h>

#define streql(a,b) (!strcmp((a),(b)))

void print_help() {
    printf("yapc [device] [action] [option]\n");
    printf("Examples:\n");
    printf("\tyapc sink list\n");
    printf("\tyapc sink list detailed\n");
}

