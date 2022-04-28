#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "fib_userspace.h"

#define FIB_DEV "/dev/fibonacci"
#define BUFFSIZE 40000

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

int compare(const void *a, const void *b)
{
    long long A = *(long long *) a;
    long long B = *(long long *) b;
    return (A > B) - (A < B);
}

int main()
{
    char buf[BUFFSIZE];
#define iter 201
#define step 1

    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    long long utime_arr[iter + 2][10];
    // char str_buf[BUFFSIZE];
    for (int i = 0; i < iter; i++) {
        for (int j = 0; j < 10; j++) {
            struct timespec start, end;
            lseek(fd, i * step, SEEK_SET);
            clock_gettime(CLOCK_REALTIME, &start);

            long long sz = read(fd, buf, BUFFSIZE);
            // fib_to_string(str_buf, BUFFSIZE, (unsigned long long *) buf, sz);
            // ubig *f = fib_sequence(j, BUFFSIZE);
            // fib_to_string(str_buf, BUFFSIZE, (unsigned long long *)f->cell,
            // f->size);

            clock_gettime(CLOCK_REALTIME, &end);
            // free(f); //userspace
            utime_arr[i][j] = (long long) end.tv_nsec - start.tv_nsec;
        }

        qsort((void *) &utime_arr[i][0], 10, sizeof(long long), compare);
        long long utime_avg = 0, ktime_avg = 0;
        for (int j = 2; j < 8; j++) {
            utime_avg += utime_arr[i][j];
        }
        utime_avg /= 6;
        printf("%d %lld\n", i * step, utime_avg);
    }

    close(fd);
    return 0;
}