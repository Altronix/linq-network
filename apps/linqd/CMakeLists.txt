add_executable(atxd main.c)
target_link_libraries(atxd network-static)
install(TARGETS atxd RUNTIME DESTINATION bin)
