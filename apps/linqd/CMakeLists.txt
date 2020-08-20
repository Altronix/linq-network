add_executable(atx-linqd main.c)
target_link_libraries(atx-linqd network-static)
install(TARGETS atx-linqd RUNTIME DESTINATION bin)
