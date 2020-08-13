set(LIBS ${CMAKE_THREAD_LIBS_INIT} usb-common-static common-static)
if(NOT MSVC)
  set(LIBS_STATIC ${LIBS} libusb-static udev)
  set(LIBS_SHARED ${LIBS} libusb-static udev)
else()
  set(LIBS_STATIC ${LIBS} libusb-static)
  set(LIBS_SHARED ${LIBS} libusb-static) # linking win with static
endif()
list(APPEND SOURCES linq_usbh.c io.c io_k64_mqx.c io_m5.c)
list(APPEND HEADERS linq_usbh.h io.h io_k64_mqx.h io_m5.h)
set(INSTALL_HEADERS linq_usbh.h)
set(INCS "./")

append_log_level_compiler_flags(DEFS)


if(ENABLE_TESTING)
  add_library(usbh-test-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(usbh-test-static ${LIBS_STATIC})
  target_include_directories(usbh-test-static PUBLIC "${INCS}")
  target_compile_definitions(usbh-test-static PUBLIC "LINQ_USB_STATIC")
  target_compile_definitions(usbh-test-static PUBLIC "${DEFS}")
endif()

if(BUILD_STATIC)
  add_library(usbh-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(usbh-static ${LIBS_STATIC})
  target_include_directories(usbh-static PUBLIC "${INCS}")
  target_compile_definitions(usbh-static PUBLIC "LINQ_USB_STATIC")
  target_compile_definitions(usbh-static PUBLIC "${DEFS}")
  set_target_properties(usbh-static PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linqusbh")
  install(TARGETS usbh-static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/altronix)
endif()

if(BUILD_SHARED)
  add_library(usbh-shared SHARED ${SOURCES} ${HEADERS})
  target_link_libraries(usbh-shared ${LIBS_SHARED})
  target_include_directories(usbh-shared PUBLIC "${INCS}")
  target_compile_definitions(usbh-shared PUBLIC "DLL_EXPORT")
  target_compile_definitions(usbh-shared PUBLIC "${DEFS}")
  set_target_properties(usbh-shared PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linqusbh")
  install(TARGETS usbh-shared
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/altronix)
endif()
