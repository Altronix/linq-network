set(LIBUSB_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/libusb)
set(LIBUSB_TEST_FILE ${LIBUSB_SOURCE_DIR}/README)
if(GIT_FOUND AND NOT EXISTS "${LIBUSB_TEST_FILE}")
  message(STATUS "Downloading libusb submodule")
  execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init external/libusb
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      RESULT_VARIABLE LIBUSB_GIT_CLONE_RESULT)
  message(STATUS "libusb download result: ${LIBUSB_GIT_CLONE_RESULT}")
endif()

if(NOT MSVC)
  # IE if(UNIX OR CYGWIN OR MINGW)
  ExternalProject_Add(libusb-project
    SOURCE_DIR ${LIBUSB_SOURCE_DIR}
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ./autogen.sh
    COMMAND ./configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND make
    INSTALL_COMMAND make install
    EXCLUDE_FROM_ALL true)
  find_library(LIBUSB_LIBRARY_PATH libusb NAMES libusb-1.0)
  message(STATUS "Found libusb          : ${LIBUSB_LIBRARY_PATH}")
  import_library_static(usb-1.0 libusb-static ${LIBUSB_LIBRARY_PATH})
  import_library_shared(usb-1.0 libusb-shared ${LIBUSB_LIBRARY_PATH})
  add_dependencies(libusb-static libusb-project)
else()
  set(LIBUSB_LIBRARY libusb-1.0${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(LIBUSB_LIBRARY ${CMAKE_INSTALL_PREFIX}/lib/${LIBUSB_LIBRARY})
  file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/include/libusb-1.0)
  add_custom_target(libusb-project DEPENDS ${LIBUSB_LIBRARY})
  add_custom_command( 
      OUTPUT "${LIBUSB_LIBRARY}"
      COMMAND MSBuild.exe
              -p:Configuration=Release
              -p:Platform=x64
              ${LIBUSB_SOURCE_DIR}/msvc/libusb_static_2019.vcxproj
      COMMAND ${CMAKE_COMMAND}
              -E
              copy
              "${LIBUSB_SOURCE_DIR}/x64/Release/lib/libusb-1.0.lib"
              ${LIBUSB_LIBRARY}
      COMMAND ${CMAKE_COMMAND}
              -E
              copy
              ${LIBUSB_SOURCE_DIR}/libusb/libusb.h
              ${CMAKE_INSTALL_PREFIX}/include/libusb-1.0/)
  find_library(LIBUSB_LIBRARY_PATH libusb NAMES libusb-1.0)
  message(STATUS "Found libusb          : ${LIBUSB_LIBRARY_PATH}")
  import_library_static(libusb-1.0 libusb-static ${LIBUSB_LIBRARY_PATH})
  add_dependencies(libusb-static libusb-project)
endif()
