set(LIBS common-static)
list(APPEND SOURCES wire.c)
list(APPEND HEADERS wire.h)
set(INCS "./")

append_log_level_compiler_flags(DEFS)

if(ENABLE_TESTING)
  add_library(usb-common-test-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(usb-common-test-static ${LIBS})
  target_include_directories(usb-common-test-static PUBLIC "${INCS}")
  target_compile_definitions(usb-common-test-static PUBLIC "LINQ_USB_STATIC")
  target_compile_definitions(usb-common-test-static PUBLIC "${DEFS}")
endif()

if(BUILD_STATIC)
  add_library(usb-common-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(usb-common-static ${LIBS})
  target_include_directories(usb-common-static PUBLIC "${INCS}")
  target_compile_definitions(usb-common-static PUBLIC "LINQ_USB_STATIC")
  target_compile_definitions(usb-common-static PUBLIC "${DEFS}")
  set_target_properties(usb-common-static PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linqusb")
  install(TARGETS usb-common-static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/altronix)
endif()

if(BUILD_SHARED)
  add_library(usb-common-shared SHARED ${SOURCES} ${HEADERS})
  target_link_libraries(usb-common-shared ${LIBS})
  target_include_directories(usb-common-shared PUBLIC "${INCS}")
  target_compile_definitions(usb-common-shared PUBLIC "DLL_EXPORT")
  target_compile_definitions(usb-common-shared PUBLIC "${DEFS}")
  set_target_properties(usb-common-shared PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linqusb")
  install(TARGETS usb-common-shared
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/altronix)
endif()

