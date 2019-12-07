# klib

file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/include/klib)
add_custom_command(
	OUTPUT ${CMAKE_INSTALL_PREFIX}/include/klib/khash.h ${CMAKE_INSTALL_PREFIX}/include/klib/klist.h
 	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/external/klib/khash.h ${CMAKE_INSTALL_PREFIX}/include/klib
 	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/external/klib/klist.h ${CMAKE_INSTALL_PREFIX}/include/klib)

add_custom_target(klib-install DEPENDS ${CMAKE_INSTALL_PREFIX}/include/klib/khash.h)
add_library(klib INTERFACE)
add_dependencies(klib klib-install)
target_include_directories(klib INTERFACE ${CMAKE_INSTALL_PREFIX}/include)
