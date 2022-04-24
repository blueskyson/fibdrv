#include <linux/slab.h>
#include <linux/string.h>

static inline int estimate_size(long long k)
{
    if (k <= 93)
        return 1;
    unsigned long long n = (k * 20899 - 34950) / 1926610;
    return (int) n + 1;
}

typedef struct BigN {
    int size;
    unsigned long long *cell;
} ubig;

ubig *new_ubig(int size)
{
    ubig *ptr = kmalloc(sizeof(ubig), GFP_KERNEL);
    if (!ptr)
        return NULL;

    unsigned long long *ullptr =
        kmalloc(size * sizeof(unsigned long long), GFP_KERNEL);
    if (!ullptr) {
        kfree(ptr);
        return NULL;
    }
    memset(ullptr, 0, size * sizeof(unsigned long long));

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
    memset(x->cell, 0, x->size * sizeof(unsigned long long));
}

static inline void ubig_assign(ubig *dest, ubig *src)
{
    memcpy(dest->cell, src->cell, dest->size * sizeof(unsigned long long));
}