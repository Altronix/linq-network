add_executable(linqd main.c)
target_link_libraries(linqd network-static)
install(TARGETS linqd RUNTIME DESTINATION bin)
