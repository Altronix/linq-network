# --------------------------------------------------------------------------
# Build test-containers
# --------------------------------------------------------------------------

find_package(Threads)

add_executable(test-containers main.c)

target_link_libraries(test-containers
                      network-test-static
                      mock-utils
                      cmocka)
add_test(NAME test-containers COMMAND test-containers)
install(TARGETS test-containers DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
