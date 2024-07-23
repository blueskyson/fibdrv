#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define FIB_DEV "/dev/fibonacci"
#define BUFFSIZE 2500
#define FIBSIZE 256

int main()
{
    char buf[BUFFSIZE];
    int N = 300; /* TODO: try test something bigger than the limit */

    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    for (int i = 0; i <= N; i++) {
        lseek(fd, i, SEEK_SET);
        long long sz = read(fd, buf, BUFFSIZE);
        if (sz <= 0) {
            printf("Error reading from " FIB_DEV " at offset %d.\n", i);
        } else {
            printf("Reading from " FIB_DEV
                   " at offset %d, returned the sequence "
                   "%s.\n",
                   i, buf);
        }
    }

    close(fd);
    return 0;
}
