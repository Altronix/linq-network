set(sources
    base64.c
    linq_io.c
    device.c
    node.c
    jsmn.c
    sys.c)
set(headers
    ../include/altronix/linq_io.h
    base64.h
    containers.h
    linq_io_internal.h
    device.h
    node.h)

set(linq_io_INSTALL_HEADERS ../include/altronix/linq_io.h)
set(linq_io_SOURCES ${sources})
set(linq_io_HEADERS ${headers})

set(linq_io_win_SOURCES ./win/sys.c)
set(linq_io_win_HEADERS ./win/sys.h)
set(linq_io_unix_SOURCES ./unix/sys.c)
set(linq_io_unix_HEADERS ./unix/sys.h)
set(linq_io_mock_SOURCES ./mock/sys.c)
set(linq_io_mock_HEADERS ./mock/sys.h)

if(NOT MSVC)
  add_library(linq-io
              ${linq_io_SOURCES}
              ${linq_io_unix_SOURCES}
              ${linq_io_HEADERS}
              ${linq_io_unix_HEADERS})
  target_link_libraries(linq-io
                        czmq
                        zmq
                        jsmn
                        klib
                        ${CMAKE_THREAD_LIBS_INIT}
                        rt
                        m
                        stdc++
                        uuid)
  target_include_directories(linq-io PUBLIC ../include)
  target_include_directories(linq-io PUBLIC ./unix)
else()
  add_definitions(-DCZMQ_STATIC)
  add_definitions(-DZMQ_STATIC)
  set(win-libs iphlpapi wldap32)
  add_library(linq-io
              ${linq_io_SOURCES}
              ${linq_io_win_SOURCES}
              ${linq_io_HEADERS}
              ${linq_io_win_HEADERS})
  target_link_libraries(linq-io
                        czmq
                        libzmq
                        jsmn
                        klib
                        ${win-libs}
                        ${CMAKE_THREAD_LIBS_INIT}
                        uuid
                        Rpcrt4)
  target_include_directories(linq-io PUBLIC ../include)
  target_include_directories(linq-io PUBLIC ./win)
endif()

if(ENABLE_TESTING)
  set(CMAKE_BUILD_TYPE Debug)
  add_library(linq-io-test
              ${linq_io_SOURCES}
              ${linq_io_mock_SOURCES}
              ${linq_io_HEADERS}
              ${linq_io_mock_HEADERS})
  target_link_libraries(linq-io-test
                        czmq
                        zmq
                        jsmn
                        klib
                        ${CMAKE_THREAD_LIBS_INIT}
                        rt
                        m
                        stdc++
                        uuid)
  target_include_directories(linq-io-test PUBLIC ../include)
  target_include_directories(linq-io-test PUBLIC ./)
  target_include_directories(linq-io-test PUBLIC ./mock)
endif()

set_target_properties(linq-io PROPERTIES PUBLIC_HEADER "${linq_io_INSTALL_HEADERS}")
install(TARGETS linq-io
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/altronix)
