add_executable(atx-tty-request main.c)
target_link_libraries(atx-tty-request network-static)
install(TARGETS atx-tty-request RUNTIME DESTINATION bin)
