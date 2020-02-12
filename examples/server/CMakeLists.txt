# --------------------------------------------------------------------------
# Build server
# --------------------------------------------------------------------------

set(WWW_ROOT_DIR ${CMAKE_SOURCE_DIR}/examples/server/www)

find_package(Threads)

add_executable(server main.c)

target_link_libraries(server linq-network-static)
target_compile_definitions(server PRIVATE "-DWWW_ROOT_DIR=\"${WWW_ROOT_DIR}\"")
install(TARGETS server DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
