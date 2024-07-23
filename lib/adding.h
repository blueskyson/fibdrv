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
