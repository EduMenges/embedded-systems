#include <cstdint>
#include <cstddef>

/**
 * @brief Hash using funny-falcon's GoodOAAT hash (https://github.com/rurban/smhasher/blob/master/Hashes.cpp)
 * @param data Pointer to data
 * @param len Data length **in_bytes**
 * @param seed Hash's seed
 * @return uint32_t The hash of data
 */
uint32_t hash(const void *data, size_t len, uint32_t seed = 0);
