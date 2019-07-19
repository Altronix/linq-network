#--------------------------------------------------------------------------
# Build test-nodes
#--------------------------------------------------------------------------

find_package(Threads)

add_executable(test-nodes main.c)

target_link_libraries(test-nodes linq-c-test helpers cmocka)
add_test(NAME test-nodes COMMAND test-nodes)
install(TARGETS test-nodes DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
