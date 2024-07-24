#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define FIB_DEV "/dev/fibonacci"
#define BUFFSIZE 2500
#define FIBSIZE 256

int fib_to_string(char *buf,
                  int buf_sz,
                  const unsigned long long *fib,
                  int fib_sz)
{
    memset(buf, '0', buf_sz);
    buf[buf_sz - 1] = '\0';

    int index = fib_sz - 1;
    while (index > 0 && !fib[index])
        index--;
    if (index < 0)
        return buf_sz - 2;

    for (int i = index; i >= 0; i--) {
        for (unsigned long long mask = 0x8000000000000000ULL; mask;
             mask >>= 1) {
            int carry = ((fib[i] & mask) != 0);
            for (int j = buf_sz - 2; j >= 0; j--) {
                buf[j] += buf[j] - '0' + carry;
                carry = (buf[j] > '9');
                if (carry)
                    buf[j] -= 10;
            }
        }
    }

    int offset = 0;
    while (buf[offset] == '0')
        offset++;
    return (buf[offset] == '\0') ? (offset - 1) : offset;
}

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
            int __offset = fib_to_string(str_buf, BUFFSIZE,
                                         (unsigned long long *) buf, sz);
            printf("Reading from " FIB_DEV
                   " at offset %d, returned the sequence "
                   "%s.\n",
                   i, str_buf + __offset);
        }
    }

    close(fd);
    return 0;
}
