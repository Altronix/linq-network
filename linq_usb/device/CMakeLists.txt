set(LIBS_STATIC "${LIBS}" linq-utils-static)
set(LIBS_SHARED "${LIBS}" linq-utils-shared)
list(APPEND SOURCES linq_usbd.c)
list(APPEND HEADERS linq_usbd.h)
set(INSTALL_HEADERS linq_usbd.h)
set(INCS "./")

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
  target_include_directories(linq-usbd-static PUBLIC "${INCS}")
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
  target_include_directories(linq-usbd-shared PUBLIC "${INCS}")
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
