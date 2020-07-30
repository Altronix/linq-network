add_executable(linqd main.c)
target_link_libraries(linqd linq-http-static linq-daemon-static)
install(TARGETS linqd RUNTIME DESTINATION bin)
