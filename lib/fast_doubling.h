#include <linux/slab.h>
#include <linux/string.h>

static inline int estimate_size(long long k)
{
    if (k <= 43)
        return 1;
    unsigned long long n = (k * 20899 - 34950) / 963305;
    return (int) n + 1;
}

typedef struct BigN {
    int size;
    unsigned int *cell;
} ubig;

ubig *new_ubig(int size)
{
    ubig *ptr = kmalloc(sizeof(ubig), GFP_KERNEL);
    if (!ptr)
        return NULL;

    unsigned int *cellptr = kmalloc(size * sizeof(unsigned int), GFP_KERNEL);
    if (!cellptr) {
        kfree(ptr);
        return NULL;
    }
    memset(cellptr, 0, size * sizeof(unsigned int));

    ptr->size = size;
    ptr->cell = cellptr;
    return ptr;
}

static inline void destroy_ubig(ubig *ptr)
{
    if (ptr) {
        if (ptr->cell)
            kfree(ptr->cell);
        kfree(ptr);
    }
}

static inline void zero_ubig(ubig *x)
{
    memset(x->cell, 0, x->size * sizeof(unsigned int));
}

static inline void ubig_assign(ubig *dest, ubig *src)
{
    memcpy(dest->cell, src->cell, dest->size * sizeof(unsigned int));
}

static inline void ubig_add(ubig *dest, ubig *a, ubig *b)
{
    for (int i = 0; i < a->size; i++)
        dest->cell[i] = a->cell[i] + b->cell[i];

    for (int i = 0; i < a->size - 1; i++)
        dest->cell[i + 1] += (dest->cell[i] < a->cell[i]);
}

static inline void ubig_sub(ubig *dest, ubig *a, ubig *b)
{
    for (int i = 0; i < a->size; i++)
        dest->cell[i] = a->cell[i] - b->cell[i];

    for (int i = 0; i < a->size - 1; i++)
        dest->cell[i + 1] -= (dest->cell[i] > a->cell[i]);
}

static inline void ubig_lshift(ubig *dest, ubig *a, int x)
{
    zero_ubig(dest);

    // quotient and remainder of x being divided by 32
    unsigned quotient = x >> 5, remainder = x & 0x1f;

    for (int i = 0; i + quotient < a->size; i++)
        dest->cell[i + quotient] |= a->cell[i] << remainder;

    if (remainder)
        for (int i = 1; i + quotient < a->size; i++)
            dest->cell[i + quotient] |= a->cell[i - 1] >> (32 - remainder);
}

static inline void ubig_mul(ubig *dest,
                            ubig *a,
                            const ubig *b,
                            ubig *shift_buf,
                            ubig *add_buf)
{
    zero_ubig(dest);
    int index = a->size - 1;
    while (index >= 0 && !b->cell[index])
        index--;
    if (index < 0)
        return;

    for (int i = index; i >= 0; i--) {
        int bit_index = (i << 5) + 31;
        for (unsigned long long mask = 0x80000000ULL; mask; mask >>= 1) {
            if (b->cell[i] & mask) {
                zero_ubig(shift_buf);
                zero_ubig(add_buf);

                ubig_lshift(shift_buf, a, bit_index);
                ubig_add(add_buf, dest, shift_buf);
                ubig_assign(dest, add_buf);
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
static ubig *fib_sequence(long long k)
{
    if (k <= 1LL) {
        ubig *result = new_ubig(1);
        if (!result)
            return NULL;
        result->cell[0] = (unsigned int) k;
        return result;
    }

    int sz = estimate_size(k);
    ubig *a = new_ubig(sz);
    ubig *b = new_ubig(sz);
    ubig *tmp1 = new_ubig(sz);
    ubig *tmp2 = new_ubig(sz);
    ubig *t1 = new_ubig(sz);
    ubig *t2 = new_ubig(sz);
    ubig *mul_buf1 = new_ubig(sz);
    ubig *mul_buf2 = new_ubig(sz);
    if (!a || !b || !tmp1 || !tmp2 || !t1 || !t2 || !mul_buf1 || !mul_buf2) {
        destroy_ubig(a);
        destroy_ubig(b);
        destroy_ubig(tmp1);
        destroy_ubig(tmp2);
        destroy_ubig(t1);
        destroy_ubig(t2);
        destroy_ubig(mul_buf1);
        destroy_ubig(mul_buf2);
        return NULL;
    }
    b->cell[0] = 1ULL;

    for (unsigned int mask = 0x80000000U >> __builtin_clzll(k); mask;
         mask >>= 1) {
        ubig_lshift(tmp1, b, 1);                    // tmp1 = 2*b
        ubig_sub(tmp2, tmp1, a);                    // tmp2 = 2*b - a
        ubig_mul(t1, a, tmp2, mul_buf1, mul_buf2);  // t1 = a*(2*b - a)

        ubig_mul(tmp1, a, a, mul_buf1, mul_buf2);  // tmp1 = a^2
        ubig_mul(tmp2, b, b, mul_buf1, mul_buf2);  // tmp2 = b^2
        ubig_add(t2, tmp1, tmp2);                  // t2 = a^2 + b^2

        ubig_assign(a, t1);
        ubig_assign(b, t2);
        if (k & mask) {
            ubig_add(t1, a, b);  // t1 = a + b
            ubig_assign(a, b);
            ubig_assign(b, t1);
        }
    }

    destroy_ubig(b);
    destroy_ubig(tmp1);
    destroy_ubig(tmp2);
    destroy_ubig(t1);
    destroy_ubig(t2);
    destroy_ubig(mul_buf1);
    destroy_ubig(mul_buf2);
    return a;
}
