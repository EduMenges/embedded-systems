add_library(json STATIC json.c)
add_executable(json_benchmark main.c)
target_link_libraries(json_benchmark PRIVATE json)