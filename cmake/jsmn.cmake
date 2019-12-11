# jsmn

# hack to see if we are installing with relative paths
STRING(SUBSTRING ${CMAKE_CURRENT_BINARY_DIR} 0 8 SUB_CURRENT_BINARY_DIR)
STRING(SUBSTRING ${CMAKE_INSTALL_PREFIX} 0 8 SUB_CMAKE_INSTALL_PREFIX)
if(${SUB_CURRENT_BINARY_DIR} STREQUAL ${SUB_CMAKE_INSTALL_PREFIX})
  set(jsmn_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/jsmn)
else()
  set(jsmn_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}/include/jsmn)
endif()
MESSAGE(STATUS "jsmn_INSTALL_DIR ${jsmn_INSTALL_DIR}")

file(MAKE_DIRECTORY ${jsmn_INSTALL_DIR})
add_custom_command(OUTPUT ${jsmn_INSTALL_DIR}/jsmn.h
 	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/external/jsmn/jsmn.h ${jsmn_INSTALL_DIR}/)

add_custom_target(jsmn-install DEPENDS ${jsmn_INSTALL_DIR}/jsmn.h)
add_library(jsmn INTERFACE)
add_dependencies(jsmn jsmn-install)
target_include_directories(jsmn INTERFACE ${jsmn_INSTALL_DIR})
