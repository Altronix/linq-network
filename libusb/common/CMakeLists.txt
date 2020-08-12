set(LIBS utils-static)
list(APPEND SOURCES wire.c)
list(APPEND HEADERS wire.h)
set(INCS "./")

append_log_level_compiler_flags(DEFS)

if(ENABLE_TESTING)
  add_library(linq-usb-common-test-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(linq-usb-common-test-static ${LIBS})
  target_include_directories(linq-usb-common-test-static PUBLIC "${INCS}")
  target_compile_definitions(linq-usb-common-test-static PUBLIC "LINQ_USB_STATIC")
  target_compile_definitions(linq-usb-common-test-static PUBLIC "${DEFS}")
endif()

if(BUILD_STATIC)
  add_library(linq-usb-common-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(linq-usb-common-static ${LIBS})
  target_include_directories(linq-usb-common-static PUBLIC "${INCS}")
  target_compile_definitions(linq-usb-common-static PUBLIC "LINQ_USB_STATIC")
  target_compile_definitions(linq-usb-common-static PUBLIC "${DEFS}")
  set_target_properties(linq-usb-common-static PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linq-usb")
  install(TARGETS linq-usb-common-static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/altronix)
endif()

if(BUILD_SHARED)
  add_library(linq-usb-common-shared SHARED ${SOURCES} ${HEADERS})
  target_link_libraries(linq-usb-common-shared ${LIBS})
  target_include_directories(linq-usb-common-shared PUBLIC "${INCS}")
  target_compile_definitions(linq-usb-common-shared PUBLIC "DLL_EXPORT")
  target_compile_definitions(linq-usb-common-shared PUBLIC "${DEFS}")
  set_target_properties(linq-usb-common-shared PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linq-usb")
  install(TARGETS linq-usb-common-shared
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/altronix)
endif()

