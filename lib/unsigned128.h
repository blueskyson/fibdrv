#include <linux/kernel.h>
#include <linux/string.h>

typedef struct BigN {
    unsigned long long upper, lower;
} unsigned128;

#define P10_UINT64 10000000000000000000ULL

/**
 * fib_sequence() - Calculate the k-th Fibonacci number.
 * @k:     Index of the Fibonacci number to calculate.
 *
 * Return: The k-th Fibonacci number on success.
 */
static unsigned128 fib_sequence(long long k)
{
    if (k <= 1LL) {
        unsigned128 ret = {.upper = 0, .lower = (unsigned long long) k};
        return ret;
    }

    unsigned128 a, b, c;
    a.upper = 0;
    a.lower = 0;
    b.upper = 0;
    b.lower = 1;


    for (int i = 2; i <= k; i++) {
        c.lower = a.lower + b.lower;
        c.upper = a.upper + b.upper;
        // if lower overflows or lower >= 10^9, add a carry to upper
        if ((c.lower < a.lower) || (c.lower >= P10_UINT64)) {
            c.lower -= P10_UINT64;
            c.upper += 1;
        }
        a = b;
        b = c;
    }

    return c;
}

/**
 * fib_to_string() - Convert the k-th Fibonacci number into string.
 * @buf: Buffer to store the resulting string.
 * @buf_sz: Size of the buffer.
 * @fib: Pointer to the 128-bit Fibonacci number structure.
 * @fib_sz: Size of the Fibonacci number structure (unused in this
 * implementation).
 *
 * This function converts a 128-bit Fibonacci number into its string
 * representation. It first initializes the buffer with '0' characters
 * and null-terminates it. Then, it uses snprintf() to convert the number
 * to a string, handling both the case where the upper 64 bits are
 * non-zero and where only the lower 64 bits are used.
 *
 * Return: The start position (offset) of the @fib string, which is always 0
 * in this implementation.
 */
int fib_to_string(char *buf, int buf_sz, void *fib, int fib_sz)
{
    const unsigned128 *f = (unsigned128 *) fib;
    if (f->upper) {
        snprintf(buf, buf_sz, "%llu%llu", f->upper, f->lower);
    } else {
        snprintf(buf, buf_sz, "%llu", f->lower);
    }

    return 0;
}