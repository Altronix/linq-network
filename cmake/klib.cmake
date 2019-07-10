# klib

file(MAKE_DIRECTORY ${deps_INSTALL_DIR}/include/klib)
add_custom_target(klib-install 
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/external/klib/khash.h ${deps_INSTALL_DIR}/include/klib)

add_library(klib INTERFACE)
add_dependencies(klib klib-install)
target_include_directories(klib INTERFACE ${deps_INSTALL_DIR}/include)
