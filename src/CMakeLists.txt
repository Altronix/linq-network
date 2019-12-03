set(sources
    base64.c
    linq_netw.c
    device.c
    node.c
    jsmn.c
    sys.c)
set(headers
    ../include/altronix/linq_netw.h
    base64.h
    containers.h
    linq_netw_internal.h
    device.h
    node.h)

set(linq_netw_INSTALL_HEADERS ../include/altronix/linq_netw.h)
set(linq_netw_SOURCES ${sources})
set(linq_netw_HEADERS ${headers})

set(linq_netw_win_SOURCES ./win/sys.c)
set(linq_netw_win_HEADERS ./win/sys.h)
set(linq_netw_unix_SOURCES ./unix/sys.c)
set(linq_netw_unix_HEADERS ./unix/sys.h)
set(linq_netw_mock_SOURCES ./mock/sys.c)
set(linq_netw_mock_HEADERS ./mock/sys.h)

if(NOT MSVC)
  add_library(linq-netw
              ${linq_netw_SOURCES}
              ${linq_netw_unix_SOURCES}
              ${linq_netw_HEADERS}
              ${linq_netw_unix_HEADERS})
  target_link_libraries(linq-netw
                        czmq
                        zmq
                        jsmn
                        klib
                        ${CMAKE_THREAD_LIBS_INIT}
                        rt
                        m
                        stdc++
                        uuid)
  target_include_directories(linq-netw PUBLIC ../include)
  target_include_directories(linq-netw PUBLIC ./unix)
else()
  add_definitions(-DCZMQ_STATIC)
  add_definitions(-DZMQ_STATIC)
  set(win-libs ws2_32 iphlpapi wldap32)
  add_library(linq-netw
              ${linq_netw_SOURCES}
              ${linq_netw_win_SOURCES}
              ${linq_netw_HEADERS}
              ${linq_netw_win_HEADERS})
  target_link_libraries(linq-netw
                        czmq
                        zmq
                        jsmn
                        klib
                        ${win-libs}
                        ${CMAKE_THREAD_LIBS_INIT}
                        uuid
                        Rpcrt4)
  target_include_directories(linq-netw PUBLIC ../include)
  target_include_directories(linq-netw PUBLIC ./win)
endif()

if(ENABLE_TESTING)
  set(CMAKE_BUILD_TYPE Debug)
  add_library(linq-netw-test
              ${linq_netw_SOURCES}
              ${linq_netw_mock_SOURCES}
              ${linq_netw_HEADERS}
              ${linq_netw_mock_HEADERS})
  target_link_libraries(linq-netw-test
                        czmq
                        zmq
                        jsmn
                        klib
                        ${CMAKE_THREAD_LIBS_INIT}
                        rt
                        m
                        stdc++
                        uuid)
  target_include_directories(linq-netw-test PUBLIC ../include)
  target_include_directories(linq-netw-test PUBLIC ./)
  target_include_directories(linq-netw-test PUBLIC ./mock)
endif()

set_target_properties(linq-netw PROPERTIES PUBLIC_HEADER "${linq_netw_INSTALL_HEADERS}")
install(TARGETS linq-netw
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/altronix)
