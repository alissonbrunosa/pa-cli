// utility function to compare strings
#define streql(a,b) (!strcmp((a),(b)))

// prints help information
void print_help();

void errorf(const char *format, ...);

char* strcopy(const char *src);

int clamp(int value, int min, int max);
