# klib

if(MSVC)
  # STRING(REPLACE ${CMAKE_CURRENT_BINARY_DIR}/ "" RINSTALL_DIR ${CMAKE_INSTALL_PREFIX})
  # set(klib_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${RINSTALL_DIR}/include/klib)
  set(klib_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/klib)
  MESSAGE(STATUS "klib_INSTALL_DIR ${klib_INSTALL_DIR}")
else()
  set(klib_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/klib)
endif()

file(MAKE_DIRECTORY ${klib_INSTALL_DIR})
add_custom_command(
	OUTPUT ${klib_INSTALL_DIR}/khash.h ${klib_INSTALL_DIR}/klist.h
 	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/external/klib/khash.h ${klib_INSTALL_DIR}/
 	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/external/klib/klist.h ${klib_INSTALL_DIR}/)

add_custom_target(klib-install DEPENDS ${klib_INSTALL_DIR}/khash.h)
add_library(klib INTERFACE)
add_dependencies(klib klib-install)
target_include_directories(klib INTERFACE ${klib_INSTALL_DIR})
