#include <linux/kernel.h>
#include <linux/string.h>

#define BIGNSIZE 12

typedef struct BigN {
    unsigned long long cell[BIGNSIZE];
} ubig;

static inline void init_ubig(ubig *x)
{
    for (int i = 0; i < BIGNSIZE; x->cell[i++] = 0)
        ;
}

/**
 * ubig_add() - Add two large unsigned integers.
 * @dest: Pointer to the destination ubig where the result will be stored.
 * @a: Pointer to the first ubig.
 * @b: Pointer to the second ubig.
 *
 * This function performs addition of two large unsigned integers represented by
 * the `ubig` structures pointed to by @a and @b. The result of the addition is
 * stored in the `ubig` structure pointed to by @dest.
 */
static inline void ubig_add(ubig *dest, ubig *a, ubig *b)
{
    for (int i = 0; i < BIGNSIZE; i++)
        dest->cell[i] = a->cell[i] + b->cell[i];

    for (int i = 0; i < BIGNSIZE - 1; i++)
        dest->cell[i + 1] += (dest->cell[i] < a->cell[i]);
}

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
        init_ubig(&result);
        result.cell[0] = (unsigned long long) k;
        return result;
    }

    ubig a, b, c;
    init_ubig(&a);
    init_ubig(&b);
    b.cell[0] = 1ULL;

    for (int i = 2; i <= k; i++) {
        ubig_add(&c, &a, &b);
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

    memset(buf, '0', buf_sz);
    buf[buf_sz - 1] = '\0';
    int index = BIGNSIZE - 1;
    while (index >= 0 && !f->cell[index])
        index--;
    if (index == -1) {
        return buf_sz - 2;
    }

    for (int i = index; i >= 0; i--) {
        for (unsigned long long mask = 0x8000000000000000ULL; mask;
             mask >>= 1) {
            int carry = ((f->cell[i] & mask) != 0);
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