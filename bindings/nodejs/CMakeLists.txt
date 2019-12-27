cmake_minimum_required(VERSION 3.11)
project(linq-netw)

# dependency install
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
find_package(Threads)

# Compiler stuff
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE on)

###############################################################################
# node-addon-api
###############################################################################
execute_process(COMMAND node -p "require('node-addon-api').include"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE NODE_ADDON_API_DIR)
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

###############################################################################
# Build linq-netw
###############################################################################
ExternalProject_Add(linq-netw-project
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../
	INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
	UPDATE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND
		cmake
		--build .
		--target install
		--config Release
	LIST_SEPARATOR |
	CMAKE_ARGS 
		-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

ExternalProject_Get_Property(linq-netw-project install_dir)
set(linq_network_INCLUDE_DIR ${install_dir}/include)
FILE(MAKE_DIRECTORY ${install_dir}/include)
add_library(linq-netw STATIC IMPORTED)
add_dependencies(linq-netw linq-netw-project)
set_target_properties(linq-netw PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${linq_network_INCLUDE_DIR})

if(NOT MSVC)
set_property(TARGET linq-netw 
  PROPERTY IMPORTED_LOCATION 
  ${install_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}linq-netw${CMAKE_STATIC_LIBRARY_SUFFIX})
else()
set_property(TARGET linq-netw 
  PROPERTY IMPORTED_LOCATION 
  ${install_dir}/lib/liblinq-netw${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()

###############################################################################
# Build linq-netw.hpp
###############################################################################
STRING(SUBSTRING ${CMAKE_CURRENT_BINARY_DIR} 0 8 SUB_CURRENT_BINARY_DIR)
STRING(SUBSTRING ${CMAKE_INSTALL_PREFIX} 0 8 SUB_CMAKE_INSTALL_PREFIX)
if(${SUB_CURRENT_BINARY_DIR} STREQUAL ${SUB_CMAKE_INSTALL_PREFIX})
  set(linq_netw_cpp_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/altronix)
else()
  set(linq_netw_cpp_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}/include/altronix)
endif()
file(MAKE_DIRECTORY ${linq_netw_cpp_INSTALL_DIR})
add_custom_command(
	OUTPUT ${linq_netw_cpp_INSTALL_DIR}/linq_netw.hpp
 	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/../cpp/linq_netw.hpp ${linq_netw_cpp_INSTALL_DIR}/)
add_custom_target(linq_netw_cpp-install DEPENDS ${linq_netw_cpp_INSTALL_DIR}/linq_netw.hpp)
add_library(linq-netw-cpp INTERFACE)
add_dependencies(linq-netw-cpp linq_netw_cpp-install)
target_include_directories(linq-netw-cpp INTERFACE ${linq_netw_cpp_INSTALL_DIR})

###############################################################################
# Build linq-netw-js
###############################################################################
add_library(linq-netw-js SHARED 
  ./src/binding.cc
  ./src/linqnetwork.h
  ./src/linqnetwork.cc)
set_target_properties(linq-netw-js PROPERTIES PREFIX "" SUFFIX ".node")
target_include_directories(linq-netw-js PRIVATE ${CMAKE_JS_INC} ${NODE_ADDON_API_DIR})
target_link_libraries(linq-netw-js linq-netw linq-netw-cpp ${CMAKE_JS_LIB})
