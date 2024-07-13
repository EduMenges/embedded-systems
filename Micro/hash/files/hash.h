#include <stddef.h>

#define uint32_t unsigned int
#define uint64_t unsigned long long int
#define uint8_t  unsigned char
#define size_t   unsigned char

/**
 * @brief Hash using funny-falcon's GoodOAAT hash (https://github.com/rurban/smhasher/blob/master/Hashes.cpp)
 * @param data Pointer to data
 * @param len Data length **in_bytes**
 * @param seed Hash's seed
 * @return uint32_t The hash of data
 */
uint32_t hash(void *d, size_t len, uint32_t seed);
