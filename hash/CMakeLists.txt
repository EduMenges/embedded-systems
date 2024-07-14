add_library(hash STATIC hash.c)
add_executable(hash_benchmark benchmark.c)
target_link_libraries(hash_benchmark PRIVATE hash)