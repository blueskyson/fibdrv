#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define FIB_DEV "/dev/fibonacci"

typedef struct BigN {
    unsigned long long upper, lower;
} u128;

void print_fib_u128(int i, u128 *fib)
{
    if (fib->upper) {
        printf("Reading from " FIB_DEV
               " at offset %d, returned the sequence "
               "%llu%019llu.\n",
               i, fib->upper, fib->lower);
    } else {
        printf("Reading from " FIB_DEV
               " at offset %d, returned the sequence "
               "%llu.\n",
               i, fib->lower);
    }
}

#define BIGN
int main()
{
    long long sz;

    char buf[sizeof(u128)];
    char write_buf[] = "testing writing";
    int offset = 100; /* TODO: try test something bigger than the limit */

    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    for (int i = 0; i <= offset; i++) {
        sz = write(fd, write_buf, strlen(write_buf));
        printf("Writing to " FIB_DEV ", returned the sequence %lld\n", sz);
    }

    for (int i = 0; i <= offset; i++) {
        lseek(fd, i, SEEK_SET);
        sz = read(fd, buf, sizeof(u128));
#ifdef BIGN
        print_fib_u128(i, (u128 *) buf);
#else
        printf("Reading from " FIB_DEV
               " at offset %d, returned the sequence "
               "%lld.\n",
               i, sz);
#endif
    }

    for (int i = offset; i >= 0; i--) {
        lseek(fd, i, SEEK_SET);
        sz = read(fd, buf, sizeof(u128));
#ifdef BIGN
        print_fib_u128(i, (u128 *) buf);
#else
        printf("Reading from " FIB_DEV
               " at offset %d, returned the sequence "
               "%lld.\n",
               i, sz);
#endif
    }

    close(fd);
    return 0;
}
