#include "hash.h"

namespace
{
inline uint32_t rotl32(uint32_t v, uint8_t n)
{
    return (v << n) | (v >> ((-n) & 31));
}

inline uint32_t rotr32(uint32_t v, uint8_t n)
{
    return (v >> n) | (v << ((-n) & 31));
}
}

uint32_t hash(const uint8_t *str, size_t len, uint32_t seed)
{
    const uint8_t *const end = str + len;
    uint32_t h1 = seed ^ 0x3b00;
    uint32_t h2 = rotl32(seed, 15);

    for (; str != end; str++)
    {
        h1 += str[0];
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
