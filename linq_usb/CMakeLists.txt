set(LIBS_STATIC "${LIBS}" linq-utils-static)
set(LIBS_SHARED "${LIBS}" linq-utils-shared)

if("${USB_SUPPORT}" STREQUAL "DEVICE")
  MESSAGE(STATUS "-- USB_SUPPORT: [DEVICE]")
  list(APPEND DEFS "-DLINQ_USB_SUPPORT=DEVICE")
  list(APPEND SOURCES device/linq_usb.c)
  list(APPEND HEADERS device/linq_usb.h)
  set(INSTALL_HEADERS device/linq_usb.h)
elseif("${USB_SUPPORT}" STREQUAL "HOST")
  MESSAGE(STATUS "-- USB_SUPPORT: [HOST]")
  list(APPEND DEFS "-DLINQ_USB_SUPPORT=HOST")
  list(APPEND SOURCES host/linq_usb.c)
  list(APPEND HEADERS host/linq_usb.h)
  set(INSTALL_HEADERS host/linq_usb.h)
  # APPEND libusb lib
else()
  message(ERROR "-- USB Support not selected!")
endif()

append_log_level_compiler_flags(DEFS)

if(ENABLE_TESTING)
  list(APPEND INCS_TEST_PUBLIC "./" "./device" "./host")
  list(APPEND INCS_TEST_PRIVATE "${INCS_TEST_PUBLIC}")
  list(APPEND DEFS_TEST_PUBLIC  "LINQ_USB_STATIC" "${DEFS}")
  list(APPEND DEFS_TEST_PRIVATE "LINQ_USB_STATIC" "${DEFS}")
  add_library(linq-usb-test-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(linq-usb-test-static ${LIBS_STATIC})
  target_include_directories(linq-usb-test-static PUBLIC ${INCS_TEST_PUBLIC})
  target_include_directories(linq-usb-test-static PRIVATE ${INCS_TEST_PRIVATE})
  target_compile_definitions(linq-usb-test-static PUBLIC "${DEFS_TEST_PUBLIC}")
  target_compile_definitions(linq-usb-test-static PRIVATE "${DEFS_TEST_PRIVATE}")
endif()

if(BUILD_STATIC)
  list(APPEND INCS_STATIC_PUBLIC "./" )
  list(APPEND INCS_STATIC_PRIVATE "")
  list(APPEND DEFS_STATIC_PUBLIC  "LINQ_USB_STATIC" "${DEFS}")
  list(APPEND DEFS_STATIC_PRIVATE "LINQ_USB_STATIC" "${DEFS}")
  add_library(linq-usb-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(linq-usb-static ${LIBS_STATIC})
  target_include_directories(linq-usb-static PUBLIC ${INCS_STATIC_PUBLIC})
  target_include_directories(linq-usb-static PRIVATE ${INCS_STATIC_PRIVATE})
  target_compile_definitions(linq-usb-static PUBLIC "${DEFS_STATIC_PUBLIC}")
  target_compile_definitions(linq-usb-static PRIVATE "${DEFS_STATIC_PRIVATE}")
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
  list(APPEND INCS_SHARED_PUBLIC "./" )
  list(APPEND INCS_SHARED_PRIVATE "")
  list(APPEND DEFS_SHARED_PUBLIC  "LINQ_USB_SHARED" "DLL_EXPORT" "${DEFS}")
  list(APPEND DEFS_SHARED_PRIVATE "LINQ_USB_SHARED" "DLL_EXPORT" "${DEFS}")
  add_library(linq-usb-shared SHARED ${SOURCES} ${HEADERS})
  target_link_libraries(linq-usb-shared ${LIBS_SHARED})
  target_include_directories(linq-usb-shared PUBLIC ${INCS_SHARED_PUBLIC})
  target_include_directories(linq-usb-shared PRIVATE ${INCS_SHARED_PRIVATE})
  target_compile_definitions(linq-usb-shared PUBLIC "${DEFS_SHARED_PUBLIC}")
  target_compile_definitions(linq-usb-shared PRIVATE "${DEFS_SHARED_PRIVATE}")
  set_target_properties(linq-usb-shared PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linq-usb")
  install(TARGETS linq-usb-shared
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/altronix)
endif()
