set(SOURCES
    linq_network.c
    device.c
    node.c
    zmtp.c)
  set(HEADERS
    linq_network.h
    linq_network_internal.h
    device.h
    node.h
    zmtp.h)

set(INSTALL_HEADERS linq_network.h)
set(LIBS ${CMAKE_THREAD_LIBS_INIT})
set(LIBS_STATIC ${LIBZMQ_STATIC_LIBS} linq-utils-static)
set(LIBS_SHARED ${LIBZMQ_SHARED_LIBS} linq-utils-shared)

append_log_level_compiler_flags(definitions)

if(NOT MSVC)
  list(APPEND LIBS rt m stdc++ uuid dl)
else()
  list(APPEND definitions "-DCZMQ_STATIC")
  list(APPEND definitions "-DZMQ_STATIC")
  list(APPEND LIBS ws2_32 iphlpapi wldap32 uuid Rpcrt4)
endif()

if(ENABLE_TESTING)
  add_library(linq-network-test-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(linq-network-test-static ${LIBS_STATIC} ${LIBS})
  target_compile_definitions(linq-network-test-static PUBLIC ${definitions})
  target_compile_definitions(linq-network-test-static PRIVATE "-DTESTING")
  target_include_directories(linq-network-test-static PUBLIC ${INCS})
  target_include_directories(linq-network-test-static PUBLIC ./)
endif()

if(BUILD_STATIC)
  add_library(linq-network-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(linq-network-static ${LIBS_STATIC} ${LIBS})
  target_compile_definitions(linq-network-static PUBLIC ${definitions})
  target_include_directories(linq-network-static PUBLIC ${INCS})
  target_include_directories(linq-network-static PUBLIC ./)
  set_target_properties(linq-network-static PROPERTIES 
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linq-network")
  install(TARGETS linq-network-static
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          RUNTIME DESTINATION bin
          PUBLIC_HEADER DESTINATION include/altronix)
endif()

if(BUILD_SHARED)
  add_library(linq-network-shared SHARED ${SOURCES} ${HEADERS})
  # if(MSVC)
  #   #TODO - https://stackoverflow.com/questions/33062728/cmake-link-shared-library-on-windows/41618677
  #   target_link_libraries(linq-network-shared ${LIBS_STATIC} ${LIBS})
  # else()
  #   target_link_libraries(linq-network-shared ${LIBS_SHARED} ${LIBS})
  # endif()
  target_link_libraries(linq-network-shared ${LIBS_SHARED} ${LIBS})
  target_compile_definitions(linq-network-shared PUBLIC ${definitions})
  target_include_directories(linq-network-shared PUBLIC ${INCS})
  target_include_directories(linq-network-shared PUBLIC ./)
  set_target_properties(linq-network-shared PROPERTIES 
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linq-network")
  install(TARGETS linq-network-shared
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          RUNTIME DESTINATION bin
          PUBLIC_HEADER DESTINATION include/altronix)
endif()
