set(LIBS_STATIC "${LIBS}" linq-utils-static)
set(LIBS_SHARED "${LIBS}" linq-utils-shared)

if("${USB_SUPPORT}" STREQUAL "DEVICE")
  MESSAGE(STATUS "USB_SUPPORT: [DEVICE]")
  list(APPEND DEFS "-DLINQ_USB_SUPPORT=DEVICE")
  list(APPEND SOURCES device/linq_usb.c)
  list(APPEND HEADERS device/linq_usb.h)
  set(INSTALL_HEADERS device/linq_usb.h)
  set(INCS "./device")
elseif("${USB_SUPPORT}" STREQUAL "HOST")
  MESSAGE(STATUS "USB_SUPPORT: [HOST]")
  list(APPEND DEFS "-DLINQ_USB_SUPPORT=HOST")
  list(APPEND SOURCES host/linq_usb.c)
  list(APPEND HEADERS host/linq_usb.h)
  set(INSTALL_HEADERS host/linq_usb.h)
  set(INCS "./host")
  # APPEND libusb lib
else()
  message(ERROR "USB Support not selected!")
endif()

append_log_level_compiler_flags(DEFS)

if(ENABLE_TESTING)
  add_library(linq-usb-test-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(linq-usb-test-static ${LIBS_STATIC})
  target_include_directories(linq-usb-test-static PUBLIC "${INCS}")
  target_compile_definitions(linq-usb-test-static PUBLIC "LINQ_USB_STATIC")
  target_compile_definitions(linq-usb-test-static PUBLIC "${DEFS}")
endif()

if(BUILD_STATIC)
  add_library(linq-usb-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(linq-usb-static ${LIBS_STATIC})
  target_include_directories(linq-usb-static PRIVATE "${INCS}")
  target_compile_definitions(linq-usb-static PUBLIC "LINQ_USB_STATIC")
  target_compile_definitions(linq-usb-static PUBLIC "${DEFS}")
  set_target_properties(linq-usb-static PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linq-usb")
  install(TARGETS linq-usb-static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/altronix)
endif()

if(BUILD_SHARED)
  add_library(linq-usb-shared SHARED ${SOURCES} ${HEADERS})
  target_link_libraries(linq-usb-shared ${LIBS_SHARED})
  target_include_directories(linq-usb-shared PRIVATE "${INCS}")
  target_compile_definitions(linq-usb-shared PUBLIC "DLL_EXPORT")
  target_compile_definitions(linq-usb-shared PUBLIC "${DEFS}")
  set_target_properties(linq-usb-shared PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linq-usb")
  install(TARGETS linq-usb-shared
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/altronix)
endif()
