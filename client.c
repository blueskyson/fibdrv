#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// First method: use struct BigN to store 128-bit unisgned integer.
// #include "lib/unsigned128.h"

// Second method: use unsigned long long array to store big number.
#include "lib/unsigned_bignum.h"

#define FIB_DEV "/dev/fibonacci"
#define FIBSIZE 256
#define BUFFSIZE 2500

int main()
{
    char buf[FIBSIZE];
    char str_buf[BUFFSIZE];
    int N = 300; /* TODO: try test something bigger than the limit */

    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    for (int i = 0; i <= N; i++) {
        lseek(fd, i, SEEK_SET);
        long long sz = read(fd, buf, FIBSIZE);
        if (sz < 0) {
            printf("Error reading from " FIB_DEV " at offset %d.\n", i);
        } else {
            int __offset = fib_to_string(str_buf, BUFFSIZE, (void *) buf, sz);
            printf("Reading from " FIB_DEV
                   " at offset %d, returned the sequence "
                   "%s.\n",
                   i, str_buf + __offset);
        }
    }

    close(fd);
    return 0;
}
