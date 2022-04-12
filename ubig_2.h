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

static inline void ubig_add(ubig *dest, ubig *a, ubig *b)
{
    for (int i = 0; i < a->size; i++)
        dest->cell[i] = a->cell[i] + b->cell[i];

    for (int i = 0; i < a->size - 1; i++)
        dest->cell[i + 1] += (dest->cell[i] < a->cell[i]);
}

inline void ubig_sub(ubig *dest, ubig *a, ubig *b)
{
    for (int i = 0; i < a->size; i++)
        dest->cell[i] = a->cell[i] - b->cell[i];

    for (int i = 0; i < a->size - 1; i++)
        dest->cell[i + 1] -= (dest->cell[i] > a->cell[i]);
}

inline void ubig_lshift(ubig *dest, ubig *a, int x)
{
    // quotient and remainder of x being divided by 64
    unsigned quotient = x >> 6, remainder = x & 0x3f;

    zero_ubig(dest);
    for (int i = 0; i + quotient < a->size; i++)
        dest->cell[i + quotient] |= a->cell[i] << remainder;

    if (remainder)
        for (int i = 1; i + quotient < a->size; i++)
            dest->cell[i + quotient] |= a->cell[i - 1] >> (64 - remainder);
}

void ubig_mul(ubig *dest, ubig *a, ubig *b, ubig *shift_buf, ubig *add_buf)
{
    zero_ubig(dest);
    int index = a->size - 1;
    while (index >= 0 && !b->cell[index])
        index--;
    if (index < 0)
        return;

    for (int i = index; i >= 0; i--) {
        int bit_index = (i << 6) + 63;
        for (unsigned long long mask = 0x8000000000000000ULL; mask;
             mask >>= 1) {
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

int ubig_to_string(char *buf, size_t bufsize, ubig *a)
{
    memset(buf, '0', bufsize);
    buf[bufsize - 1] = '\0';

    int index = a->size - 1;
    while (index > 0 && !a->cell[index])
        index--;
    if (index < 0)
        return bufsize - 2;

    for (int i = index; i >= 0; i--) {
        for (unsigned long long mask = 0x8000000000000000ULL; mask;
             mask >>= 1) {
            int carry = ((a->cell[i] & mask) != 0);
            for (int j = bufsize - 2; j >= 0; j--) {
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

//#define ADDING
#define FAST_DOUBLING

#ifdef ADDING
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

    for (unsigned long long mask = 0x8000000000000000ULL >> __builtin_clzll(k);
         mask; mask >>= 1) {
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
#endif