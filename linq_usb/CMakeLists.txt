set(LIBS_STATIC "${LIBS}" linq-utils-static)
set(LIBS_SHARED "${LIBS}" linq-utils-shared)

if("${USB_SUPPORT}" STREQUAL "DEVICE")
  MESSAGE(STATUS "USB_SUPPORT: [DEVICE]")
  list(APPEND DEFS "-DLINQ_USB_SUPPORT=DEVICE")
  list(APPEND SOURCES device/linq_usbd.c)
  list(APPEND HEADERS device/linq_usbd.h)
  set(INSTALL_HEADERS device/linq_usbd.h)
  set(INCS "./device")
elseif("${USB_SUPPORT}" STREQUAL "HOST")
  MESSAGE(STATUS "USB_SUPPORT: [HOST]")
  list(APPEND DEFS "-DLINQ_USB_SUPPORT=HOST")
  list(APPEND SOURCES host/linq_usbh.c)
  list(APPEND HEADERS host/linq_usbh.h)
  set(INSTALL_HEADERS host/linq_usbh.h)
  set(INCS "./host")
  # APPEND libusb lib
else()
  message(ERROR "USB Support not selected!")
endif()

append_log_level_compiler_flags(DEFS)

if(ENABLE_TESTING)
  add_library(linq-usbd-test-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(linq-usbd-test-static ${LIBS_STATIC})
  target_include_directories(linq-usbd-test-static PUBLIC "${INCS}")
  target_compile_definitions(linq-usbd-test-static PUBLIC "LINQ_USB_STATIC")
  target_compile_definitions(linq-usbd-test-static PUBLIC "${DEFS}")
endif()

if(BUILD_STATIC)
  add_library(linq-usbd-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(linq-usbd-static ${LIBS_STATIC})
  target_include_directories(linq-usbd-static PRIVATE "${INCS}")
  target_compile_definitions(linq-usbd-static PUBLIC "LINQ_USB_STATIC")
  target_compile_definitions(linq-usbd-static PUBLIC "${DEFS}")
  set_target_properties(linq-usbd-static PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linq-usb")
  install(TARGETS linq-usbd-static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/altronix)
endif()

if(BUILD_SHARED)
  add_library(linq-usbd-shared SHARED ${SOURCES} ${HEADERS})
  target_link_libraries(linq-usbd-shared ${LIBS_SHARED})
  target_include_directories(linq-usbd-shared PRIVATE "${INCS}")
  target_compile_definitions(linq-usbd-shared PUBLIC "DLL_EXPORT")
  target_compile_definitions(linq-usbd-shared PUBLIC "${DEFS}")
  set_target_properties(linq-usbd-shared PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linq-usb")
  install(TARGETS linq-usbd-shared
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/altronix)
endif()
