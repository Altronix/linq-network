# jsmn

if(MSVC)
  # STRING(REPLACE ${CMAKE_CURRENT_BINARY_DIR}/ "" RINSTALL_DIR ${CMAKE_INSTALL_PREFIX})
  # set(jsmn_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${RINSTALL_DIR}/include/jsmn)
  set(jsmn_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/jsmn)
  MESSAGE(STATUS "jsmn_INSTALL_DIR ${jsmn_INSTALL_DIR}")
else()
  set(jsmn_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/jsmn)
endif()

file(MAKE_DIRECTORY ${jsmn_INSTALL_DIR})
add_custom_command(OUTPUT ${jsmn_INSTALL_DIR}/jsmn.h
 	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/external/jsmn/jsmn.h ${jsmn_INSTALL_DIR}/)

add_custom_target(jsmn-install DEPENDS ${jsmn_INSTALL_DIR}/jsmn.h)
add_library(jsmn INTERFACE)
add_dependencies(jsmn jsmn-install)
target_include_directories(jsmn INTERFACE ${jsmn_INSTALL_DIR})
