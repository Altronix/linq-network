set(LIBS ${CMAKE_THREAD_LIBS_INIT} linq-usb-common-static linq-utils-static)
if(NOT MSVC)
  list(APPEND LIBS udev)
endif()
set(LIBS_STATIC ${LIBS} libusb-static)
set(LIBS_SHARED ${LIBS} libusb-static)
list(APPEND SOURCES linq_usbh.c io.c io_k64_mqx.c io_m5.c)
list(APPEND HEADERS linq_usbh.h io.h io_k64_mqx.h io_m5.h)
set(INSTALL_HEADERS linq_usbh.h)
set(INCS "./")

append_log_level_compiler_flags(DEFS)


if(ENABLE_TESTING)
  add_library(linq-usbh-test-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(linq-usbh-test-static ${LIBS_STATIC})
  target_include_directories(linq-usbh-test-static PUBLIC "${INCS}")
  target_compile_definitions(linq-usbh-test-static PUBLIC "LINQ_USB_STATIC")
  target_compile_definitions(linq-usbh-test-static PUBLIC "${DEFS}")
endif()

if(BUILD_STATIC)
  add_library(linq-usbh-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(linq-usbh-static ${LIBS_STATIC})
  target_include_directories(linq-usbh-static PUBLIC "${INCS}")
  target_compile_definitions(linq-usbh-static PUBLIC "LINQ_USB_STATIC")
  target_compile_definitions(linq-usbh-static PUBLIC "${DEFS}")
  set_target_properties(linq-usbh-static PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linq-usbh")
  install(TARGETS linq-usbh-static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/altronix)
endif()

if(BUILD_SHARED)
  add_library(linq-usbh-shared SHARED ${SOURCES} ${HEADERS})
  target_link_libraries(linq-usbh-shared ${LIBS_SHARED})
  target_include_directories(linq-usbh-shared PUBLIC "${INCS}")
  target_compile_definitions(linq-usbh-shared PUBLIC "DLL_EXPORT")
  target_compile_definitions(linq-usbh-shared PUBLIC "${DEFS}")
  set_target_properties(linq-usbh-shared PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linq-usbh")
  install(TARGETS linq-usbh-shared
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/altronix)
endif()
