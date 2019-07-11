set(sources 
	linq.c
	device.c
	device_map.c)
set(headers 
	../include/altronix/linq.h
	linq_internal.h
	device.h
	device_map.h)

set(linq_SOURCES ${sources})
set(linq_HEADERS ${headers})

# when linking on windows/unix/test
add_library(linq-unix ./unix/sys.c ./unix/sys.h)
add_library(linq-win ./win/sys.c ./win/sys.h)
add_library(linq-mock ./mock/sys.c ./mock/sys.h)
target_include_directories(linq-unix PUBLIC ./unix)
target_include_directories(linq-win PUBLIC ./win)
target_include_directories(linq-mock PUBLIC ./mock)

add_library(linq-c ${linq_SOURCES} ${linq_HEADERS})
target_link_libraries(linq-c linq-unix czmq zmq jsmn klib ${CMAKE_THREAD_LIBS_INIT} rt m stdc++ uuid)
target_include_directories(linq-c PUBLIC ../include)

set(CMAKE_BUILD_TYPE Debug)
add_library(linq-c-test ${linq_SOURCES} ${linq_HEADERS})
target_link_libraries(linq-c-test linq-mock czmq zmq jsmn klib ${CMAKE_THREAD_LIBS_INIT} rt m stdc++ uuid)
target_include_directories(linq-c-test PUBLIC ../include)
target_include_directories(linq-c-test PUBLIC ./)
target_include_directories(linq-c-test PUBLIC ./mock)
