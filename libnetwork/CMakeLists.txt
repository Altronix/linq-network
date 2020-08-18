set(SOURCES
    callbacks.c
    netw.c
    node.c
    zmtp.c
    zmtp_device.c)
  set(HEADERS
    callbacks.h
    netw.h
    netw_internal.h
    node.h
    zmtp.h
    zmtp_device.h)
set(INSTALL_HEADERS netw.h)
set(INCS ../)
if(BUILD_LINQD)
  list(APPEND SOURCES 
    ./routes/alerts.c
    ./routes/devices.c
    ./routes/proxy.c
    ./routes/users.c)
  list(APPEND HEADERS ./routes/routes.h)
  list(APPEND LIBS ${JSMN_WEB_TOKENS_LIBS} database-static http-static)
  list(APPEND definitions "BUILD_LINQD")
endif()
list(APPEND LIBS ${CMAKE_THREAD_LIBS_INIT} ${ZMQ_LIBS} common-static)

append_log_level_compiler_flags(definitions)

if(NOT MSVC)
  list(APPEND LIBS rt m stdc++ uuid dl)
else()
  list(APPEND definitions "-DCZMQ_STATIC")
  list(APPEND definitions "-DZMQ_STATIC")
  list(APPEND LIBS ws2_32 iphlpapi wldap32 uuid Rpcrt4)
endif()

if(ENABLE_TESTING)
  add_library(network-test-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(network-test-static mock-czmq ${LIBS})
  target_compile_definitions(network-test-static PUBLIC 
    "LINQ_STATIC" 
    ${definitions})
  set_target_properties(network-test-static PROPERTIES 
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linqnetwork-test")
  target_compile_definitions(network-test-static PRIVATE "-DTESTING")
  target_include_directories(network-test-static PUBLIC ${INCS})
  target_include_directories(network-test-static PUBLIC ./)
endif()

if(BUILD_STATIC)
  add_library(network-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(network-static ${LIBS})
  target_compile_definitions(network-static PUBLIC 
    "LINQ_STATIC" 
    ${definitions})
  target_include_directories(network-static PUBLIC ${INCS})
  target_include_directories(network-static PUBLIC ./)
  set_target_properties(network-static PROPERTIES 
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linqnetwork")
  install(TARGETS network-static
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          RUNTIME DESTINATION bin
          PUBLIC_HEADER DESTINATION include/altronix)
endif()

if(BUILD_SHARED)
  add_library(network-shared SHARED ${SOURCES} ${HEADERS})
  target_link_libraries(network-shared ${LIBS})
  target_compile_definitions(network-shared PUBLIC 
    "DLL_EXPORT" 
    ${definitions})
  target_include_directories(network-shared PUBLIC ${INCS})
  target_include_directories(network-shared PUBLIC ./)
  set_target_properties(network-shared PROPERTIES 
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linqnetwork")
  install(TARGETS network-shared
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          RUNTIME DESTINATION bin
          PUBLIC_HEADER DESTINATION include/altronix)
endif()
