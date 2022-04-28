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

// modify for dest, src having different size
static inline void ubig_assign(ubig *dest, ubig *src)
{
    int sz = dest->size < src->size ? dest->size : src->size;
    memset(dest->cell, 0, dest->size * sizeof(unsigned int));
    memcpy(dest->cell, src->cell, sz * sizeof(unsigned int));
}

// modify for a, b having different size
static inline void ubig_add(ubig *dest, ubig *a, ubig *b)
{
    for (int i = 0; i < a->size; i++)
        dest->cell[i] = a->cell[i] + b->cell[i];

    for (int i = 0; i < a->size - 1; i++)
        dest->cell[i + 1] += (dest->cell[i] < a->cell[i]);
}

// modified addition for karatsuba
static inline void ubig_add_in_place(ubig *dest, ubig *src, int front, int end)
{
    int carry = 0, i = 0;
    for (int j = front; j < end; i++, j++) {
        unsigned int tmp = dest->cell[i] + src->cell[j] + carry;
        carry = (tmp < dest->cell[i]);
        dest->cell[i] = tmp;
    }

    for (; i < dest->size; i++) {
        unsigned int tmp = dest->cell[i] + carry;
        carry = (tmp < dest->cell[i]);
        dest->cell[i] = tmp;
    }
}

// modified addition for karatsuba
static inline void ubig_add_in_place2(ubig *dest, int offset, ubig *src)
{
    int carry = 0, i = offset;
    for (int j = 0; j < src->size; i++, j++) {
        unsigned int tmp = dest->cell[i] + src->cell[j] + carry;
        carry = (tmp < dest->cell[i]);
        dest->cell[i] = tmp;
    }

    for (; i < dest->size; i++) {
        unsigned int tmp = dest->cell[i] + carry;
        carry = (tmp < dest->cell[i]);
        dest->cell[i] = tmp;
    }
}

static inline void ubig_sub(ubig *dest, ubig *a, ubig *b)
{
    for (int i = 0; i < a->size; i++)
        dest->cell[i] = a->cell[i] - b->cell[i];

    for (int i = 0; i < a->size - 1; i++)
        dest->cell[i + 1] -= (dest->cell[i] > a->cell[i]);
}

// modified substraction for karatsuba
static inline void ubig_sub_in_place(ubig *dest, ubig *src, int front, int end)
{
    int carry = 0, i = 0;
    for (int j = front; j < src->size && j < end; i++, j++) {
        unsigned int tmp = dest->cell[i] - src->cell[j] - carry;
        carry = (tmp > dest->cell[i]);
        dest->cell[i] = tmp;
    }

    for (; i < dest->size; i++) {
        unsigned int tmp = dest->cell[i] - carry;
        carry = (tmp > dest->cell[i]);
        dest->cell[i] = tmp;
    }
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

int mul_recursive(ubig *dest, ubig *x, ubig *y, int front, int end)
{
    // termination 32 bit x 32 bit case
    int size = end - front;
    if (size == 1) {
        unsigned long long product =
            (unsigned long long) x->cell[front] * y->cell[front];
        if (front * 2 < dest->size) {
            dest->cell[front * 2] = product;
            if (front * 2 + 1 < dest->size)
                dest->cell[front * 2 + 1] = product >> 32;
        }
        return 1;
    }

    // dest = z2 * 2^(middle * 2) + z0 * 2^(front * 2)
    int half_size = size / 2;
    int middle = front + half_size;
    int ret;
    ret = mul_recursive(dest, x, y, middle, end);
    if (!ret)
        return 0;
    ret = mul_recursive(dest, x, y, front, middle);
    if (!ret)
        return 0;

    // tmp1 = (x0 + x1) and tmp2 = (y0 + y1)
    ubig *tmp1 = new_ubig(half_size + 2);
    ubig *tmp2 = new_ubig(half_size + 2);
    ubig *z1 = new_ubig((half_size + 2) * 2);
    if (!tmp1 || !tmp2 || !z1) {
        destroy_ubig(tmp1);
        destroy_ubig(tmp2);
        destroy_ubig(z1);
        return 0;
    }
    memcpy(tmp1->cell, x->cell + middle, (end - middle) * sizeof(unsigned int));
    memcpy(tmp2->cell, y->cell + middle, (end - middle) * sizeof(unsigned int));
    ubig_add_in_place(tmp1, x, front, middle);  // x0 + x1
    ubig_add_in_place(tmp2, y, front, middle);  // y0 + y1

    // z1 = (tmp1 * tmp2) - z0 - z2
    int sz_1 = ubig_msb_idx(tmp1) + 1;
    int sz_2 = ubig_msb_idx(tmp2) + 1;
    int common_sz = sz_1 > sz_2 ? sz_1 : sz_2;
    ret = mul_recursive(z1, tmp1, tmp2, 0, common_sz);
    destroy_ubig(tmp1);
    destroy_ubig(tmp2);
    if (!ret) {
        destroy_ubig(z1);
        return 0;
    }
    ubig_sub_in_place(z1, dest, front * 2, front * 2 + half_size * 2);
    ubig_sub_in_place(z1, dest, front * 2 + half_size * 2,
                      front * 2 + size * 2);

    // dest = dest + z1 * 2^(front + middle)
    ubig_add_in_place2(dest, front * 2 + half_size, z1);
    destroy_ubig(z1);
    return 1;
}

int ubig_mul(ubig *dest, ubig *a, ubig *b)
{
    zero_ubig(dest);

    // don't use karatsuba if dest only has 32 bit
    if (dest->size == 1) {
        dest->cell[0] = a->cell[0] * b->cell[0];
        return 1;
    }

    // find how many unsigned int are actually
    // storing data
    int sz_a = ubig_msb_idx(a) + 1;
    int sz_b = ubig_msb_idx(b) + 1;

    // if a == 0 or b == 0 then dest = 0
    if (sz_a == 0 || sz_b == 0)
        return 1;

    int common_sz = sz_a > sz_b ? sz_a : sz_b;
    return mul_recursive(dest, a, b, 0, common_sz);
}

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

    int ret = 1;
    for (unsigned long long mask = 0x8000000000000000ULL >> __builtin_clzll(k);
         mask; mask >>= 1) {

        ubig_lshift(tmp1, b, 1);      // tmp1 = 2*b
        ubig_sub(tmp2, tmp1, a);      // tmp2 = 2*b - a
        ret = ubig_mul(t1, a, tmp2);  // t1 = a*(2*b - a)
        if (!ret)
            break;

        ret = ubig_mul(tmp1, a, a);  // tmp1 = a^2
        if (!ret)
            break;

        ret = ubig_mul(tmp2, b, b);  // tmp2 = b^2
        if (!ret)
            break;

        ubig_add(t2, tmp1, tmp2);  // t2 = a^2 + b^2

        ubig_assign(a, t1);
        ubig_assign(b, t2);
        if (k & mask) {
            ubig_add(t1, a, b);  // t1 = a + b
            ubig_assign(a, b);
            ubig_assign(b, t1);
        }
    }

    if (!ret) {
        destroy_ubig(a);
        a = NULL;
    }

    destroy_ubig(b);
    destroy_ubig(tmp1);
    destroy_ubig(tmp2);
    destroy_ubig(t1);
    destroy_ubig(t2);
    return a;
}
