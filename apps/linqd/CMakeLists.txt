add_executable(linqd main.c)
target_link_libraries(linqd http-static daemon-static)
install(TARGETS linqd RUNTIME DESTINATION bin)
