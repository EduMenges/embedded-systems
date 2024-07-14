add_library(json STATIC json.c)
add_executable(benchmark main.c)
target_link_libraries(benchmark PRIVATE json)