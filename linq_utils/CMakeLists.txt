# Main source files
set(SOURCES base64.c)
set(HEADERS base64.h klib/khash.h klib/klist.h containers.h log.h)

if(NOT MSVC)
  list(APPEND LIBS rt m stdc++ uuid dl)
  list(APPEND INCS ./sys/unix)
  list(APPEND SOURCES sys/unix/sys.c)
  list(APPEND HEADERS sys/unix/sys.h)
else()
  list(APPEND LIBS ws2_32 iphlpapi wldap32 uuid Rpcrt4)
  list(APPEND INCS ./sys/win)
  list(APPEND SOURCES sys/win/sys.c)
  list(APPEND HEADERS sys/win/sys.h)
endif()

list(APPEND INCS ./)
set(LIBS_STATIC ${LIBS})
set(LIBS_SHARED ${LIBS})

# Build a static library
add_library(linq-utils-static STATIC ${SOURCES} ${HEADERS})
target_link_libraries(linq-utils-static ${LIBS_STATIC})
target_include_directories(linq-utils-static PUBLIC ${INCS})
set_target_properties(linq-utils-static PROPERTIES
  PUBLIC_HEADER "${HEADERS}"
  OUTPUT_NAME "linq-utils")
install(TARGETS linq-utils-static
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/altronix)

# Build a shared library
add_library(linq-utils-shared SHARED ${SOURCES} ${HEADERS})
target_link_libraries(linq-utils-shared ${LIBS_SHARED})
target_include_directories(linq-utils-shared PUBLIC ${INCS})
set_target_properties(linq-utils-shared PROPERTIES
  PUBLIC_HEADER "${HEADERS}"
  OUTPUT_NAME "linq-utils")
install(TARGETS linq-utils-shared
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/altronix)
