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
    ubig *ptr = (ubig *) kmalloc(sizeof(ubig), GFP_KERNEL);
    if (!ptr)
        return (ubig *) NULL;

    unsigned long long *ullptr = (unsigned long long *) kmalloc(
        size * sizeof(unsigned long long), GFP_KERNEL);
    if (!ullptr) {
        kfree(ptr);
        return (ubig *) NULL;
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
            return (ubig *) NULL;
        ret->cell[0] = (unsigned long long) k;
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
        return (ubig *) NULL;
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

/**
 * fib_to_string() - Convert the k-th Fibonacci number into string.
 * @buf: Buffer where the resulting string will be stored.
 * @buf_sz: Size of the buffer.
 * @fib: Pointer to the Fibonacci number data.
 *
 * Return: The start position (offset) of the @fib string.
 */
int fib_to_string(char *buf, int buf_sz, ubig *fib)
{
    memset(buf, '0', buf_sz);
    buf[buf_sz - 1] = '\0';
    int index = fib->size - 1;
    while (index >= 0 && !fib->cell[index])
        index--;
    if (index == -1) {
        return buf_sz - 2;
    }

    for (int i = index; i >= 0; i--) {
        for (unsigned long long mask = 0x8000000000000000ULL; mask;
             mask >>= 1) {
            int carry = ((fib->cell[i] & mask) != 0);
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