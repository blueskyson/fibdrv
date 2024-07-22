#include <linux/kernel.h>
#include <linux/string.h>

#define BIGNSIZE 12

typedef struct BigN {
    unsigned long long cell[BIGNSIZE];
} ubig;

#define init_ubig(x) for (int i = 0; i < BIGNSIZE; x.cell[i++] = 0)

#define P10_UINT64 10000000000000000000ULL

/**
 * fib_sequence() - Calculate the k-th Fibonacci number.
 * @k:     Index of the Fibonacci number to calculate.
 *
 * Return: The k-th Fibonacci number on success.
 */
static ubig fib_sequence(long long k)
{
    if (k <= 1LL) {
        ubig result;
        init_ubig(result);
        result.cell[0] = (unsigned long long) k;
        return result;
    }

    ubig a, b, c;
    init_ubig(a);
    init_ubig(b);
    b.cell[0] = 1;

    for (int i = 2; i <= k; i++) {
        for (int j = 0; j < BIGNSIZE; j++)
            c.cell[j] = a.cell[j] + b.cell[j];

        for (int j = 0; j < BIGNSIZE - 1; j++) {
            // if lower overflows or lower >= 10^9, add a carry to upper
            if ((c.cell[j] < a.cell[j]) || (c.cell[j] >= P10_UINT64)) {
                c.cell[j] -= P10_UINT64;
                c.cell[j + 1] += 1;
            }
        }

        a = b;
        b = c;
    }

    return c;
}

/**
 * fib_to_string() - Convert the k-th Fibonacci number into string.
 * @buf: Buffer where the resulting string will be stored.
 * @buf_sz: Size of the buffer.
 * @fib: Pointer to the Fibonacci number data.
 * @fib_sz: Size of the Fibonacci number data (unused in this implementation).
 *
 * Return: The start position (offset) of the @fib string, which is always 0
 * in this implementation.
 */
int fib_to_string(char *buf, int buf_sz, void *fib, int fib_sz)
{
    const ubig *f = (ubig *) fib;

    memset(buf, '\0', buf_sz);
    int index = BIGNSIZE - 1;
    while (index >= 0 && !f->cell[index])
        index--;
    if (index == -1) {
        buf[0] = '0';
        buf[1] = '\0';
        return 0;
    }

    char buf2[22];
    snprintf(buf2, 22, "%llu", f->cell[index--]);
    strcat(buf, buf2);
    while (index >= 0) {
        snprintf(buf2, 22, "%019llu", f->cell[index--]);
        strcat(buf, buf2);
    }
    return 0;
}