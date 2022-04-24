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

    unsigned int *ullptr = kmalloc(size * sizeof(unsigned int), GFP_KERNEL);
    if (!ullptr) {
        kfree(ptr);
        return NULL;
    }
    memset(ullptr, 0, size * sizeof(unsigned int));

    ptr->size = size;
    ptr->cell = ullptr;
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

static inline int ubig_msb_idx(ubig *a)
{
    int msb_i = a->size - 1;
    while (msb_i >= 0 && !a->cell[msb_i])
        msb_i--;
    return msb_i;
}

void ubig_mul(ubig *dest, ubig *a, ubig *b)
{
    zero_ubig(dest);

    // find the array index of the MSB of a, b
    int msb_a = ubig_msb_idx(a);
    int msb_b = ubig_msb_idx(b);
    // a == 0 or b == 0
    if (msb_a < 0 || msb_b < 0)
        return;

    // calculate the length of linear convolution vector
    int length = msb_a + msb_b + 1;

    /* do linear convolution */
    unsigned long long carry = 0ULL;
    for (int i = 0; i < length; i++) {
        unsigned long long row_sum = carry;
        carry = 0;

        int end = (i <= msb_b) ? i : msb_b;
        int start = i - end;
        for (int j = start, k = end; j <= end; j++, k--) {
            unsigned long long product =
                (unsigned long long) a->cell[k] * b->cell[j];
            row_sum += product;
            carry += (row_sum < product);
        }
        dest->cell[i] = (unsigned int) row_sum;
        carry = (carry << 32) + (row_sum >> 32);
    }

    dest->cell[length] = carry;
}

// #define ADDING
#define FAST_DOUBLING

#ifdef ADDING
static ubig *fib_sequence(long long k, size_t user_size)
{
    if (k <= 1LL) {
        if (user_size < sizeof(unsigned int))
            return NULL;

        ubig *ret = new_ubig(1);
        if (!ret)
            return NULL;
        ret->cell[0] = (unsigned int) k;
        return ret;
    }

    int sz = estimate_size(k);
    if (user_size < sz * sizeof(unsigned int))
        return NULL;

    ubig *a = new_ubig(sz);
    ubig *b = new_ubig(sz);
    ubig *c = new_ubig(sz);
    if (!a || !b || !c) {
        destroy_ubig(a);
        destroy_ubig(b);
        destroy_ubig(c);
        return NULL;
    }

    b->cell[0] = 1ULL;
    for (int i = 2; i <= k; i++) {
        ubig_add(c, a, b);
        ubig_assign(a, b);
        ubig_assign(b, c);
    }

    destroy_ubig(a);
    destroy_ubig(b);
    return c;
}

#elif defined FAST_DOUBLING
static ubig *fib_sequence(long long k, size_t user_size)
{
    if (k <= 1LL) {
        if (user_size < sizeof(unsigned long long))
            return NULL;

        ubig *ret = new_ubig(1);
        if (!ret)
            return NULL;

        ret->cell[0] = (unsigned long long) k;
        return ret;
    }

    int sz = estimate_size(k);
    if (user_size < sz * sizeof(unsigned long long))
        return NULL;

    ubig *a = new_ubig(sz);
    ubig *b = new_ubig(sz);
    ubig *tmp1 = new_ubig(sz);
    ubig *tmp2 = new_ubig(sz);
    ubig *t1 = new_ubig(sz);
    ubig *t2 = new_ubig(sz);
    if (!a || !b || !tmp1 || !tmp2 || !t1 || !t2) {
        destroy_ubig(a);
        destroy_ubig(b);
        destroy_ubig(tmp1);
        destroy_ubig(tmp2);
        destroy_ubig(t1);
        destroy_ubig(t2);
        return NULL;
    }
    b->cell[0] = 1U;

    for (unsigned long long mask = 0x8000000000000000ULL >> __builtin_clzll(k);
         mask; mask >>= 1) {
        ubig_lshift(tmp1, b, 1);  // tmp1 = 2*b
        ubig_sub(tmp2, tmp1, a);  // tmp2 = 2*b - a
        ubig_mul(t1, a, tmp2);    // t1 = a*(2*b - a)

        ubig_mul(tmp1, a, a);      // tmp1 = a^2
        ubig_mul(tmp2, b, b);      // tmp2 = b^2
        ubig_add(t2, tmp1, tmp2);  // t2 = a^2 + b^2

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
    return a;
}
#endif