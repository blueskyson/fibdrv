#define BIGNSIZE 12

typedef struct BigN {
    unsigned long long cell[BIGNSIZE];
} ubig;

inline void init_ubig(ubig *x)
{
    for (int i = 0; i < BIGNSIZE; x->cell[i++] = 0)
        ;
}

inline void ubig_add(ubig *dest, ubig *a, ubig *b)
{
    for (int i = 0; i < BIGNSIZE; i++)
        dest->cell[i] = a->cell[i] + b->cell[i];

    for (int i = 0; i < BIGNSIZE - 1; i++)
        dest->cell[i + 1] += (dest->cell[i] < a->cell[i]);
}

inline void ubig_sub(ubig *dest, ubig *a, ubig *b)
{
    for (int i = 0; i < BIGNSIZE; i++)
        dest->cell[i] = a->cell[i] - b->cell[i];

    for (int i = 0; i < BIGNSIZE - 1; i++)
        dest->cell[i + 1] -= (dest->cell[i] > a->cell[i]);
}

inline void ubig_lshift(ubig *dest, ubig *a, int x)
{
    // quotient and remainder of x being divided by 64
    unsigned quotient = x >> 6, remainder = x & 0x3f;

    init_ubig(dest);
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
    while (index > 0 && !b->cell[index])
        index--;
    if (index < 0)
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

int ubig_to_string(char *buf, size_t bufsize, ubig *a)
{
    memset(buf, '0', bufsize);
    buf[bufsize - 1] = '\0';

    int index = BIGNSIZE - 1;
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