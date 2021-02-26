set(SOURCES
    netw.c
    zmtp/callbacks.c
    zmtp/node.c
    zmtp/zmtp.c
    zmtp/zmtp_device.c
    common/base64.c
    common/config.c
    common/device.c
    common/json.c
    common/log.c
    common/rlp.c)
  set(HEADERS
    netw.h
    netw_internal.h
    zmtp/callbacks.h
    zmtp/node.h
    zmtp/zmtp.h
    zmtp/zmtp_device.h
    common/base64.h 
    common/config.h
    common/containers.h
    common/device.h
    common/json.h 
    common/jsmn.h 
    common/log.h
    common/klib/khash.h 
    common/klib/klist.h)
set(INSTALL_HEADERS netw.h common.h)
set(INCS ./)
if(BUILD_USBH)
  list(APPEND definitions "BUILD_USBH")
  if(NOT MSVC)
    list(APPEND LIBS libusb-static udev)
  else()
    list(APPEND LIBS libusb-static)
  endif()
  list(APPEND SOURCES 
    usb/common/wire.c
    usb/host/io.c
    usb/host/io_k64_mqx.c
    usb/host/io_m5.c
    usb/host/usbh.c)
  list(APPEND HEADERS 
    usb/common/wire.h
    usb/host/io.h
    usb/host/io_k64_mqx.h
    usb/host/io_m5.h
    usb/host/usbh.h)
endif()
if(NOT MSVC)
  list(APPEND INCS common/sys/unix)
  list(APPEND SOURCES common/sys/unix/sys.c)
  list(APPEND HEADERS common/sys/unix/sys.h)
  list(APPEND INSTALL_HEADERS common/sys/unix/sys.h)
else()
  list(APPEND INCS common/sys/win)
  list(APPEND SOURCES common/sys/win/sys.c)
  list(APPEND HEADERS common/sys/win/sys.h)
  list(APPEND INSTALL_HEADERS common/sys/win/sys.h)
endif()

list(APPEND LIBS ${CMAKE_THREAD_LIBS_INIT} ${ZMQ_LIBS})

append_log_level_compiler_flags(definitions)
append_version_compiler_flags(definitions)

if(NOT MSVC)
  list(APPEND LIBS rt m stdc++ uuid dl)
else()
  list(APPEND definitions "-DZMQ_STATIC")
  list(APPEND LIBS ws2_32 iphlpapi wldap32 uuid Rpcrt4)
endif()

if(ENABLE_TESTING)
  add_library(network-test-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(network-test-static ${LIBS})
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
