set(sources 
	linq.c
	device.c
	device_map.c)
set(headers 
	../include/altronix/linq.h
	linq_internal.h
	device.h
	device_map.h)

set(linq-c-sources ${sources})
set(linq-c-headers ${headers})


# when linking on windows/unix/test
add_library(linq-win ./win/sys.c ./win/sys.h)
target_include_directories(linq-win PUBLIC ./win)
add_library(linq-unix ./unix/sys.c ./unix/sys.h)
target_include_directories(linq-unix PUBLIC ./unix)
add_library(linq-mock ./mock/sys.c ./mock/sys.h)
target_include_directories(linq-mock PUBLIC ./mock)

add_library(linq-c ${linq-c-sources} ${linq-c-headers})
target_link_libraries(linq-c linq-unix czmq zmq klib ${CMAKE_THREAD_LIBS_INIT} rt m stdc++ uuid)
target_include_directories(linq-c PUBLIC ../include)

set(CMAKE_BUILD_TYPE Debug)
add_library(linq-c-test ${linq-c-sources} ${linq-c-headers})
target_link_libraries(linq-c-test linq-mock czmq zmq klib ${CMAKE_THREAD_LIBS_INIT} rt m stdc++ uuid)
target_include_directories(linq-c-test PUBLIC ../include)
target_include_directories(linq-c-test PUBLIC ./)
