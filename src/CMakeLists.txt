set(sources
    base64.c
    linq.c
    device.c
    node.c
    jsmn.c
    sys.c)
set(headers
    ../include/altronix/linq.h
    base64.h
    containers.h
    linq_internal.h
    device.h
    node.h)

set(linq_INSTALL_HEADERS ../include/altronix/linq.h)
set(linq_SOURCES ${sources})
set(linq_HEADERS ${headers})

set(linq_win_SOURCES ./win/sys.c)
set(linq_win_HEADERS ./win/sys.h)
set(linq_unix_SOURCES ./unix/sys.c)
set(linq_unix_HEADERS ./unix/sys.h)
set(linq_mock_SOURCES ./mock/sys.c)
set(linq_mock_HEADERS ./mock/sys.h)

if(NOT MSVC)
  add_library(linq
              ${linq_SOURCES}
              ${linq_unix_SOURCES}
              ${linq_HEADERS}
              ${linq_unix_HEADERS})
  target_link_libraries(linq
                        czmq
                        zmq
                        jsmn
                        klib
                        ${CMAKE_THREAD_LIBS_INIT}
                        rt
                        m
                        stdc++
                        uuid)
  target_include_directories(linq PUBLIC ../include)
  target_include_directories(linq PUBLIC ./unix)
else()
  add_definitions(-DCZMQ_STATIC)
  add_definitions(-DZMQ_STATIC)
  set(win-libs iphlpapi wldap32)
  add_library(linq
              ${linq_SOURCES}
              ${linq_win_SOURCES}
              ${linq_HEADERS}
              ${linq_win_HEADERS})
  target_link_libraries(linq
                        czmq
                        libzmq
                        jsmn
                        klib
                        ${win-libs}
                        ${CMAKE_THREAD_LIBS_INIT}
                        uuid
                        Rpcrt4)
  target_include_directories(linq PUBLIC ../include)
  target_include_directories(linq PUBLIC ./win)
endif()

if(ENABLE_TESTING)
  set(CMAKE_BUILD_TYPE Debug)
  add_library(linq-test
              ${linq_SOURCES}
              ${linq_mock_SOURCES}
              ${linq_HEADERS}
              ${linq_mock_HEADERS})
  target_link_libraries(linq-test
                        czmq
                        zmq
                        jsmn
                        klib
                        ${CMAKE_THREAD_LIBS_INIT}
                        rt
                        m
                        stdc++
                        uuid)
  target_include_directories(linq-test PUBLIC ../include)
  target_include_directories(linq-test PUBLIC ./)
  target_include_directories(linq-test PUBLIC ./mock)
endif()

set_target_properties(linq PROPERTIES PUBLIC_HEADER "${linq_INSTALL_HEADERS}")
install(TARGETS linq
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/altronix)
