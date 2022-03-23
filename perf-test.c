#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define FIB_DEV "/dev/fibonacci"

#define BIGN
#define BUFFSIZE 500

int compare(const void *a, const void *b)
{
    long long A = *(long long *) a;
    long long B = *(long long *) b;
    return (A > B) - (A < B);
}

int main()
{
    char buf[BUFFSIZE];
    int offset = 1000;

    int fd = open(FIB_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    long long utime_arr[1001][100];
    long long ktime_arr[1001][100];
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j <= offset; j++) {
            struct timespec start, end;
            lseek(fd, j, SEEK_SET);
            clock_gettime(CLOCK_REALTIME, &start);
            ktime_arr[j][i] = read(fd, buf, BUFFSIZE);
            clock_gettime(CLOCK_REALTIME, &end);
            utime_arr[j][i] = (long long) end.tv_nsec - start.tv_nsec;
        }
    }

    for (int i = 0; i <= 1000; i++) {
        qsort((void *) &utime_arr[i][0], 100, sizeof(long long), compare);
        qsort((void *) &ktime_arr[i][0], 100, sizeof(long long), compare);
        long long utime_avg = 0, ktime_avg = 0;
        for (int j = 10; j < 90; j++) {
            utime_avg += utime_arr[i][j];
            ktime_avg += ktime_arr[i][j];
        }
        utime_avg /= 80;
        ktime_avg /= 80;
        printf("%d %lld %lld %lld\n", i, ktime_avg, utime_avg,
               utime_avg - ktime_avg);
    }

    close(fd);
    return 0;
}