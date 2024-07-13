#include "hash.h"

static inline uint32_t rotl32(uint32_t v, uint8_t n)
{
    return (v << n) | (v >> ((-n) & 31));
}

static inline uint32_t rotr32(uint32_t v, uint8_t n)
{
    return (v >> n) | (v << ((-n) & 31));
}

uint32_t hash(const void *data, size_t len, uint32_t seed)
{
    uint32_t h1 = seed ^ 0x3b00;
    uint32_t h2 = rotl32(seed, 15);

    const uint8_t *p = data;
    size_t i;
    for (i = 0; i < len; ++i)
    {
        h1 += p[0];
        h1 *= 9;
        h2 += h1;
        h2 = rotl32(h2, 7);
        h2 *= 5;
    }

    h1 ^= h2;
    h1 += rotl32(h2, 14);
    h2 ^= h1;
    h2 += rotr32(h1, 6);
    h1 ^= h2;
    h1 += rotl32(h2, 5);
    h2 ^= h1;
    h2 += rotr32(h1, 8);

    return h2;
}
