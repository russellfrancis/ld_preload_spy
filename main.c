#include <stdio.h>

int main(int argc, char **argv) {
    printf("Calling the fopen() function.\n");
    FILE *fp = fopen("test.txt", "r");
    if (!fp) {
        printf("fopen() returned NULL.\n");
        return 1;
    }
    fclose(fp);
    printf("fopen() succeeded\n");
    return 0;
}