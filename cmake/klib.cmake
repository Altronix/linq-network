# klib

# hack to see if we are installing with relative paths
STRING(SUBSTRING ${CMAKE_CURRENT_BINARY_DIR} 0 8 SUB_CURRENT_BINARY_DIR)
STRING(SUBSTRING ${CMAKE_INSTALL_PREFIX} 0 8 SUB_CMAKE_INSTALL_PREFIX)
if(${SUB_CURRENT_BINARY_DIR} STREQUAL ${SUB_CMAKE_INSTALL_PREFIX})
  set(klib_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/klib)
else()
  set(klib_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}/include/klib)
endif()
MESSAGE(STATUS "klib_INSTALL_DIR ${klib_INSTALL_DIR}")

file(MAKE_DIRECTORY ${klib_INSTALL_DIR})
add_custom_command(
	OUTPUT ${klib_INSTALL_DIR}/khash.h ${klib_INSTALL_DIR}/klist.h
 	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/external/klib/khash.h ${klib_INSTALL_DIR}/
 	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/external/klib/klist.h ${klib_INSTALL_DIR}/)

add_custom_target(klib-install DEPENDS ${klib_INSTALL_DIR}/khash.h)
add_library(klib INTERFACE)
add_dependencies(klib klib-install)
target_include_directories(klib INTERFACE ${klib_INSTALL_DIR})
