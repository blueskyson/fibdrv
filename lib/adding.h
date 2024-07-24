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

static inline void ubig_assign(ubig *dest, const ubig *src)
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

/**
 * fib_sequence() - Calculate the k-th Fibonacci number.
 * @k:     Index of the Fibonacci number to calculate.
 *
 * Return: The k-th Fibonacci number on success.
 */
static ubig *fib_sequence(long long k)
{
    if (k <= 1LL) {
        ubig *ret = new_ubig(1);
        if (!ret)
            return NULL;
        ret->cell[0] = (unsigned int) k;
        return ret;
    }

    int sz = estimate_size(k);
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
