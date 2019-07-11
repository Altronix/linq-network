# jsmn

file(MAKE_DIRECTORY ${deps_INSTALL_DIR}/include/jsmn)
add_custom_command(OUTPUT ${deps_INSTALL_DIR}/include/jsmn/jsmn.h
 	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/external/jsmn/jsmn.h ${deps_INSTALL_DIR}/include/jsmn)

add_custom_target(jsmn-install DEPENDS ${deps_INSTALL_DIR}/include/jsmn/jsmn.h)
add_library(jsmn INTERFACE)
add_dependencies(jsmn jsmn-install)
target_include_directories(jsmn INTERFACE ${deps_INSTALL_DIR}/include)
