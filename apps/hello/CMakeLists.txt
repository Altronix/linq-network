add_executable(atx-hello main.c)
target_link_libraries(atx-hello network-static)
install(TARGETS atx-hello RUNTIME DESTINATION bin)
