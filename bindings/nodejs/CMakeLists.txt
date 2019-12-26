cmake_minimum_required(VERSION 3.11)
project(linq-netw)

# dependency install
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
find_package(Threads)

# Compiler stuff
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

###############################################################################
# Build linq-netw
###############################################################################
ExternalProject_ADD(linq-netw-project
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
		-DCMAKE_INSTALL_LIBDIR=<INSTALL_DIR>/lib
  )

ExternalProject_Get_Property(linq-netw-project install_dir)
set(linq_network_INCLUDE_DIR ${install_dir}/include)
FILE(MAKE_DIRECTORY ${install_dir}/include)
add_library(linq-netw STATIC IMPORTED)
add_dependencies(linq-netw linq-netw-project)

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
# Build linq-netw-js
###############################################################################
add_library(linq-netw-js SHARED ./src/binding.c)
set_target_properties(linq-netw-js PROPERTIES PREFIX "" SUFFIX ".node")
target_include_directories(linq-netw-js PRIVATE ${CMAKE_JS_INC})
target_link_libraries(linq-netw-js linq-netw ${CMAKE_JS_LIB})
