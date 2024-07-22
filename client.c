#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "lib/unsigned128.h"

#define FIB_DEV "/dev/fibonacci"
#define FIBSIZE 256
#define BUFFSIZE 2500

int main()
{
    long long sz;

    char buf[FIBSIZE];
    char str_buf[BUFFSIZE];
    char write_buf[] = "testing writing";
    int N = 100; /* TODO: try test something bigger than the limit */

    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    for (int i = 0; i <= N; i++) {
        sz = write(fd, write_buf, strlen(write_buf));
        printf("Writing to " FIB_DEV ", returned the sequence %lld\n", sz);
    }

    for (int i = 0; i <= N; i++) {
        lseek(fd, i, SEEK_SET);
        sz = read(fd, buf, FIBSIZE);
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

    for (int i = N; i >= 0; i--) {
        lseek(fd, i, SEEK_SET);
        sz = read(fd, buf, FIBSIZE);
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
