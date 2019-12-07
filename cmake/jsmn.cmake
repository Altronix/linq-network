# jsmn

file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/include/jsmn)
add_custom_command(OUTPUT ${CMAKE_INSTALL_PREFIX}/include/jsmn/jsmn.h
 	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/external/jsmn/jsmn.h ${CMAKE_INSTALL_PREFIX}/include/jsmn)

add_custom_target(jsmn-install DEPENDS ${CMAKE_INSTALL_PREFIX}/include/jsmn/jsmn.h)
add_library(jsmn INTERFACE)
add_dependencies(jsmn jsmn-install)
target_include_directories(jsmn INTERFACE ${CMAKE_INSTALL_PREFIX}/include)
