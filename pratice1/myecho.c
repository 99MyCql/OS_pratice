#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[]) {
    assert(argc >= 2);
    int i = 1;
    for ( ; i < argc; i++) {
        printf("%s", argv[i]);
        if (i == argc-1) printf("\n");
        else printf(" ");
    }
    return 0;
}
