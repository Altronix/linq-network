set(LIBS usb-common-static common-static)
list(APPEND SOURCES linq_usbd.c)
list(APPEND HEADERS linq_usbd.h)
set(INSTALL_HEADERS linq_usbd.h)
set(INCS "./")

append_log_level_compiler_flags(DEFS)

if(ENABLE_TESTING)
  add_library(usbd-test-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(usbd-test-static ${LIBS})
  target_include_directories(usbd-test-static PUBLIC "${INCS}")
  target_compile_definitions(usbd-test-static PUBLIC "LINQ_USB_STATIC")
  target_compile_definitions(usbd-test-static PUBLIC "${DEFS}")
endif()

if(BUILD_STATIC)
  add_library(usbd-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(usbd-static ${LIBS})
  target_include_directories(usbd-static PUBLIC "${INCS}")
  target_compile_definitions(usbd-static PUBLIC "LINQ_USB_STATIC")
  target_compile_definitions(usbd-static PUBLIC "${DEFS}")
  set_target_properties(usbd-static PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linqusbd")
  install(TARGETS usbd-static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/altronix)
endif()

if(BUILD_SHARED)
  add_library(usbd-shared SHARED ${SOURCES} ${HEADERS})
  target_link_libraries(usbd-shared ${LIBS})
  target_include_directories(usbd-shared PUBLIC "${INCS}")
  target_compile_definitions(usbd-shared PUBLIC "DLL_EXPORT")
  target_compile_definitions(usbd-shared PUBLIC "${DEFS}")
  set_target_properties(usbd-shared PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linqusbd")
  install(TARGETS usbd-shared
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/altronix)
endif()
