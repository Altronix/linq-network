add_library(atx-linqd-lib parse_config.c parse_config.h)
target_link_libraries(atx-linqd-lib network-static)

add_executable(atx-linqd main.c)
target_link_libraries(atx-linqd atx-linqd-lib network-static)
install(TARGETS atx-linqd RUNTIME DESTINATION bin)
