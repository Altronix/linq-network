set(sources
    base64.c
    linq_netw.c
    device.c
    node.c
    jsmn.c
    sys.c
    zmtp.c)
set(headers
    ../include/altronix/linq_netw.h
    base64.h
    containers.h
    linq_netw_internal.h
    device.h
    node.h
    zmtp.h)

set(linq_netw_INSTALL_HEADERS ../include/altronix/linq_netw.h)
set(linq_netw_SOURCES ${sources})
set(linq_netw_HEADERS ${headers})

set(linq_netw_win_SOURCES ./win/sys.c)
set(linq_netw_win_HEADERS ./win/sys.h)
set(linq_netw_unix_SOURCES ./unix/sys.c)
set(linq_netw_unix_HEADERS ./unix/sys.h)
set(linq_netw_mock_SOURCES ./mock/sys.c)
set(linq_netw_mock_HEADERS ./mock/sys.h)

if (${LINQ_LOG_LEVEL} STREQUAL "TRACE" OR
    ${LINQ_LOG_LEVEL} STREQUAL "DEBUG" OR
    ${LINQ_LOG_LEVEL} STREQUAL "INFO" OR
    ${LINQ_LOG_LEVEL} STREQUAL "WARN" OR
    ${LINQ_LOG_LEVEL} STREQUAL "ERROR" OR
    ${LINQ_LOG_LEVEL} STREQUAL "FATAL" OR
    ${LINQ_LOG_LEVEL} STREQUAL "NONE")
  MESSAGE(STATUS "LINQ_LOG_LEVEL set ${LINQ_LOG_LEVEL}")
  list(APPEND linq_netw_HEADERS log.h)
else()
  MESSAGE(STATUS "LINQ_LOG_LEVEL DISABLED")
endif()

if (WITH_MONGOOSE)
  MESSAGE(STATUS "MONGOOSE ENABLED")
  add_definitions(-DWITH_MONGOOSE)
  add_definitions(-DMG_ENABLE_HTTP_STREAMING_MULTIPART)
  add_definitions(-DMG_ENABLE_WEBSOCKET)
  list(APPEND linq_netw_HEADERS mongoose.h http.h)
  list(APPEND linq_netw_SOURCES mongoose.c http.c)
else()
  MESSAGE(STATUS "MONGOOSE DISABLED")
endif()

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
