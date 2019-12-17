set(sources
    base64.c
    linq_netw.c
    device.c
    node.c
    jsmn.c
    jsmn_helpers.c
    sys.c
    zmtp.c)
set(headers
    ../include/altronix/linq_netw.h
    base64.h
    containers.h
    jsmn_helpers.h
    linq_netw_internal.h
    device.h
    node.h
    zmtp.h)

set(linq_netw_INSTALL_HEADERS ../include/altronix/linq_netw.h)
set(linq_netw_SOURCES ${sources})
set(linq_netw_HEADERS ${headers})

set(linq_netw_win_SOURCES ./sys/win/sys.c)
set(linq_netw_win_HEADERS ./sys/win/sys.h)
set(linq_netw_unix_SOURCES ./sys/unix/sys.c)
set(linq_netw_unix_HEADERS ./sys/unix/sys.h)
set(linq_netw_mock_SOURCES ./sys/mock/sys.c)
set(linq_netw_mock_HEADERS ./sys/mock/sys.h)

if (${LINQ_LOG_LEVEL} STREQUAL "TRACE" OR
    ${LINQ_LOG_LEVEL} STREQUAL "DEBUG" OR
    ${LINQ_LOG_LEVEL} STREQUAL "INFO" OR
    ${LINQ_LOG_LEVEL} STREQUAL "WARN" OR
    ${LINQ_LOG_LEVEL} STREQUAL "ERROR" OR
    ${LINQ_LOG_LEVEL} STREQUAL "FATAL" OR
    ${LINQ_LOG_LEVEL} STREQUAL "NONE")
  MESSAGE(STATUS "LINQ_LOG_LEVEL set ${LINQ_LOG_LEVEL}")
  list(APPEND definitions "-DLINQ_LOG_LEVEL=${LINQ_LOG_LEVEL}")
  list(APPEND linq_netw_HEADERS log.h)
else()
  MESSAGE(STATUS "LINQ_LOG_LEVEL DISABLED")
endif()

if (WITH_MONGOOSE)
  MESSAGE(STATUS "MONGOOSE ENABLED")
  list(APPEND definitions "-DWITH_MONGOOSE")
  list(APPEND definitions "-DMG_ENABLE_HTTP_STREAMING_MULTIPART")
  list(APPEND definitions "-DMG_ENABLE_WEBSOCKET")
  list(APPEND definitions "-DMG_ENABLE_CALLBACK_USERDATA")
  list(APPEND linq_netw_HEADERS mongoose.h http.h ./routes/routes.h)
  list(APPEND linq_netw_SOURCES 
    mongoose.c 
    http.c
    ./routes/route_alerts.c
    ./routes/route_devices.c
    ./routes/route_proxy.c)
else()
  MESSAGE(STATUS "MONGOOSE DISABLED")
endif()

if (WITH_SQLITE)
  MESSAGE(STATUS "SQlite Enabled")
  list(APPEND definitions "-DSQLITE_CORE")
  list(APPEND definitions "-DSQLITE_DEFAULT_FORIEGN_KEYS=1")
  list(APPEND definitions "-DSQLITE_ENABLE_API_ARMOR")
  list(APPEND definitions "-DSQLITE_ENABLE_COLUMN_METADATA")
  list(APPEND definitions "-DSQLITE_ENABLE_DBSTAT_VTAB")
  list(APPEND definitions "-DSQLITE_ENABLE_FTS3")
  list(APPEND definitions "-DSQLITE_ENABLE_FTS3_PARENTHESIS")
  list(APPEND definitions "-DSQLITE_ENABLE_FTS5")
  list(APPEND definitions "-DSQLITE_ENABLE_JSON1")
  list(APPEND definitions "-DSQLITE_ENABLE_LOAD_EXTENSION")
  list(APPEND definitions "-DSQLITE_ENABLE_MEMORY_MANAGEMENT")
  list(APPEND definitions "-DSQLITE_ENABLE_RTREE")
  list(APPEND definitions "-DSQLITE_ENABLE_STAT2")
  list(APPEND definitions "-DSQLITE_ENABLE_STAT4")
  list(APPEND definitions "-DSQLITE_SOUNDEX")
  list(APPEND definitions "-DSQLITE_THREADSAFE=1")
  list(APPEND definitions "-DSQLITE_USE_URI")
  list(APPEND definitions "-DHAVE_USLEEP=1")
  list(APPEND linq_netw_SOURCES ./sqlite3.c ./sqlite3.h)
else()
  MESSAGE(STATUS "SQlite Disabled")
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
  target_compile_definitions(linq-netw PUBLIC ${definitions})
  target_include_directories(linq-netw PUBLIC ../include)
  target_include_directories(linq-netw PUBLIC ./sys/unix)
  target_include_directories(linq-netw PRIVATE ./)
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
  target_compile_definitions(linq-netw PUBLIC ${definitions})
  target_include_directories(linq-netw PUBLIC ../include)
  target_include_directories(linq-netw PUBLIC ./sys/win)
  target_include_directories(linq-netw PRIVATE ./)
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
  target_compile_definitions(linq-netw-test PUBLIC ${definitions})
  target_include_directories(linq-netw-test PUBLIC ../include)
  target_include_directories(linq-netw-test PUBLIC ./)
  target_include_directories(linq-netw-test PUBLIC ./sys/mock)
  target_include_directories(linq-netw-test PRIVATE ./)
endif()

set_target_properties(linq-netw PROPERTIES PUBLIC_HEADER "${linq_netw_INSTALL_HEADERS}")
install(TARGETS linq-netw
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/altronix)
