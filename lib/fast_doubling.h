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

static inline void ubig_add(ubig *dest, ubig *a, ubig *b)
{
    for (int i = 0; i < BIGNSIZE; i++)
        dest->cell[i] = a->cell[i] + b->cell[i];

    for (int i = 0; i < BIGNSIZE - 1; i++)
        dest->cell[i + 1] += (dest->cell[i] < a->cell[i]);
}

static inline void ubig_sub(ubig *dest, ubig *a, ubig *b)
{
    for (int i = 0; i < BIGNSIZE; i++)
        dest->cell[i] = a->cell[i] - b->cell[i];

    for (int i = 0; i < BIGNSIZE - 1; i++)
        dest->cell[i + 1] -= (dest->cell[i] > a->cell[i]);
}

static inline void ubig_lshift(ubig *dest, ubig *a, int x)
{
    init_ubig(dest);

    // quotient and remainder of x being divided by 64
    unsigned quotient = x >> 6, remainder = x & 0x3f;
    for (int i = 0; i + quotient < BIGNSIZE; i++)
        dest->cell[i + quotient] |= a->cell[i] << remainder;

    if (remainder)
        for (int i = 1; i + quotient < BIGNSIZE; i++)
            dest->cell[i + quotient] |= a->cell[i - 1] >> (64 - remainder);
}

void ubig_mul(ubig *dest, ubig *a, ubig *b)
{
    init_ubig(dest);
    int index = BIGNSIZE - 1;
    while (index >= 0 && !b->cell[index])
        index--;
    if (index == -1)
        return;

    for (int i = index; i >= 0; i--) {
        int bit_index = (i << 6) + 63;
        for (unsigned long long mask = 0x8000000000000000ULL; mask;
             mask >>= 1) {
            if (b->cell[i] & mask) {
                ubig shift, tmp;
                ubig_lshift(&shift, a, bit_index);
                ubig_add(&tmp, dest, &shift);
                *dest = tmp;
            }
            bit_index--;
        }
    }
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
        ubig ret;
        init_ubig(&ret);
        ret.cell[0] = (unsigned long long) k;
        return ret;
    }

    ubig a, b;
    init_ubig(&a);
    init_ubig(&b);
    b.cell[0] = 1ULL;

    for (unsigned long long mask = 0x8000000000000000ULL >> __builtin_clzll(k);
         mask; mask >>= 1) {
        ubig tmp1, tmp2, t1, t2;
        ubig_lshift(&tmp1, &b, 1);   // tmp1 = 2*b
        ubig_sub(&tmp2, &tmp1, &a);  // tmp2 = 2*b - a
        ubig_mul(&t1, &a, &tmp2);    // t1 = a*(2*b - a)

        ubig_mul(&tmp1, &a, &a);      // tmp1 = a^2
        ubig_mul(&tmp2, &b, &b);      // tmp2 = b^2
        ubig_add(&t2, &tmp1, &tmp2);  // t2 = a^2 + b^2

        a = t1, b = t2;
        if (k & mask) {
            ubig_add(&t1, &a, &b);  // t1 = a + b
            a = b, b = t1;
        }
    }

    return a;
}
