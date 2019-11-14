# --------------------------------------------------------------------------
# Build server
# --------------------------------------------------------------------------

find_package(Threads)

add_executable(server main.c)

target_link_libraries(server linq-io)
install(TARGETS server DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
