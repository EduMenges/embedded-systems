add_library(laplacian STATIC laplacian.c)
add_executable(benchmark benchmark.c)
target_link_libraries(benchmark PRIVATE hash)