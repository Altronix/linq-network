set(sources 
	linq.c
	device.c
	device_request.c
	jsmn.c)
set(headers 
	../include/altronix/linq.h
	containers.h
	linq_internal.h
	device.h
	device_request.h)

set(linq_INSTALL_HEADERS ../include/altronix/linq.h)
set(linq_SOURCES ${sources})
set(linq_HEADERS ${headers})

set(linq_win_SOURCES ./win/sys.c)
set(linq_win_HEADERS ./win/sys.h)
set(linq_unix_SOURCES ./unix/sys.c)
set(linq_unix_HEADERS ./unix/sys.h)
set(linq_mock_SOURCES ./mock/sys.c)
set(linq_mock_HEADERS ./mock/sys.h)

add_library(linq-c ${linq_SOURCES} ${linq_unix_SOURCES} ${linq_HEADERS} ${linq_unix_HEADERS})
target_link_libraries(linq-c czmq zmq jsmn klib ${CMAKE_THREAD_LIBS_INIT} rt m stdc++ uuid)
target_include_directories(linq-c PUBLIC ../include)
target_include_directories(linq-c PUBLIC ./unix)

set(CMAKE_BUILD_TYPE Debug)
add_library(linq-c-test ${linq_SOURCES} ${linq_mock_SOURCES} ${linq_HEADERS} ${linq_mock_HEADERS})
target_link_libraries(linq-c-test czmq zmq jsmn klib ${CMAKE_THREAD_LIBS_INIT} rt m stdc++ uuid)
target_include_directories(linq-c-test PUBLIC ../include)
target_include_directories(linq-c-test PUBLIC ./)
target_include_directories(linq-c-test PUBLIC ./mock)

set_target_properties(linq-c PROPERTIES PUBLIC_HEADER "${linq_INSTALL_HEADERS}")
install(TARGETS linq-c
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	PUBLIC_HEADER DESTINATION include/altronix)
