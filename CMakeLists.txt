cmake_policy(SET CMP0091 NEW)
cmake_minimum_required(VERSION 3.15...3.16)
project(linq-network)

find_package(Git)
if(NOT Git_FOUND)
  message(FATAL_ERROR "Git not found! Git is required to install submodules")
endif()

# dependency install
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
find_package(Threads)

# Compiler stuff
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(cmake/helpers.cmake)
include(cmake/config.cmake)

if(BUILD_DEBUG)
  set(CMAKE_BUILD_TYPE Debug)
endif()
# NOTE required this line here to make linking on windows easier
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# [DEPENDENCIES] libzmq, [,jsmn-web-tokens ,openssl ,libusb]
if(BUILD_DEPENDENCIES)
  if(BUILD_LINQD)
    find_package(OpenSSL REQUIRED)
    set(TLS_LIBS OpenSSL::Crypto OpenSSL::SSL)
    include(cmake/jsmn-web-tokens.cmake)
    set(JSMN_WEB_TOKENS_LIBS jsmn-web-tokens-static)
  endif()
  if(BUILD_USBH)
    include(cmake/libusb.cmake)
  endif()
  include(cmake/libzmq.cmake)
  set(ZMQ_LIBS zmq-static)
else()
  if("${BUILDROOT_DIR}" STREQUAL "")
    set(SEARCH_DIR "${CMAKE_INSTALL_PREFIX}/lib")
    if(BUILD_LINQD)
      find_package(OpenSSL REQUIRED)
      set(TLS_LIBS OpenSSL::Crypto OpenSSL::SSL)
      set(JSMN_WEB_TOKENS_LIBS jsmn-web-tokens)
    endif()
    if(BUILD_USBH)
      find_library(USB_LIBRARY_PATH usb NO_CMAKE_SYSTEM_PATH PATHS ${SEARCH_DIR})
      message(STATUS "Found usb  : ${USB_LIBRARY_PATH}")
      import_library_static(usb-1.0 libusb-static ${USB_LIBRARY_PATH})
      import_library_shared(usb-1.0 libusb-shared ${USB_LIBRARY_PATH})
    endif()
    find_library(ZMQ_LIBRARY_PATH zmq NO_CMAKE_SYSTEM_PATH PATHS ${SEARCH_DIR})
    message(STATUS "Found zmq  : ${ZMQ_LIBRARY_PATH}")
    import_library_shared(zmq zmq-shared ${ZMQ_LIBRARY_PATH})
    set(ZMQ_LIBS zmq-shared)
  else()
    # When building for buildroot outside of buildroot use BUILDROOT_DIR=/...
    message(STATUS "Building for remote buildroot")
    set(TRIPPLE "arm-buildroot-linux-gnueabihf")
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR arm)
    set(CMAKE_C_COMPILER "${BUILDROOT_DIR}/output/host/bin/${TRIPPLE}-cc")
    set(CMAKE_CXX_COMPILER "${BUILDROOT_DIR}/output/host/bin/${TRIPPLE}-c++")
    set(SEARCH_DIR "${BUILDROOT_DIR}/output/staging/usr/lib")
    set(CMAKE_PREFIX_PATH="${BUILDROT_DIR}/output/staging/usr")
    if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/include")
      FILE(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/include)
    endif()
    if(BUILD_LINQD)
      find_library(SSL_LIBRARY_PATH ssl 
        NO_CMAKE_SYSTEM_PATH PATHS 
        ${SEARCH_DIR})
      find_library(CRYPTO_LIBRARY_PATH crypto
        NO_CMAKE_SYSTEM_PATH PATHS 
        ${SEARCH_DIR})
      find_library(JSMN_WEB_TOKENS_LIBRARY_PATH jsmn-web-tokens
        NO_CMAKE_SYSTEM_PATH PATHS 
        ${SEARCH_DIR}) 
      message(STATUS "Found ssl             : ${SSL_LIBRARY_PATH}")
      message(STATUS "Found crypto          : ${CRYPTO_LIBRARY_PATH}")
      message(STATUS "Found jsmn-web-tokens : ${JSMN_WEB_TOKENS_LIBRARY_PATH}")
      import_library_shared(jsmn-web-tokens jsmn-web-tokens-shared ${JSMN_WEB_TOKENS_LIBRARY_PATH})
      import_library_shared(ssl ssl-shared ${SSL_LIBRARY_PATH})
      import_library_shared(crypto crypto-shared ${CRYPTO_LIBRARY_PATH})
      set(TLS_LIBS ssl-shared crypto-shared)
      set(JSMN_WEB_TOKENS_LIBS jsmn-web-tokens-shared)
    endif()
    find_library(ZMQ_LIBRARY_PATH zmq NO_CMAKE_SYSTEM_PATH PATHS ${SEARCH_DIR})
    message(STATUS "Found zmq             : ${ZMQ_LIBRARY_PATH}")
    import_library_shared(zmq zmq-shared ${ZMQ_LIBRARY_PATH})
    set(ZMQ_LIBS zmq-shared)
  endif()
endif()

if(ENABLE_TESTING)
  set(CMAKE_BUILD_TYPE Debug)
  set(CMAKE_EXPORT_COMPILE_COMMANDS on)
  enable_testing()
  include(CTest)
  include(cmake/cmocka.cmake)
  include(cmake/CodeCoverage.cmake)
  include(cmake/http-parser.cmake)
  if(EXISTS "${CMAKE_SOURCE_DIR}/external/linq-common/CMakeLists.txt")
    MESSAGE(STATUS "linq-common found!")
    MESSAGE(STATUS "Building linq-common integration test")
    include(cmake/wolfssl.cmake)
    include(cmake/test-fixture.cmake)
    add_subdirectory(test/integration)
    add_subdirectory(test/_fixture)
  endif()
  add_subdirectory(test/_helpers)
  add_subdirectory(test/_mocks/mock_utils)
  add_subdirectory(test/_mocks/mock_file)
  add_subdirectory(test/_mocks/mock_libusb)
  add_subdirectory(test/_mocks/mock_zmq)
  add_subdirectory(test/containers)
  add_subdirectory(test/device)
  add_subdirectory(test/devices)
  add_subdirectory(test/json)
  add_subdirectory(test/rlp)
  add_subdirectory(test/sys)
  add_subdirectory(test/usb/common)
  add_subdirectory(test/usb/host)
  # add_subdirectory(src/test/linq_network)
  # add_subdirectory(bindings/cpp/test)

  # add test coverage
  APPEND_COVERAGE_COMPILER_FLAGS()
  set(COVERAGE_EXCLUDES 
    "src/common/base64.*")

  setup_target_for_coverage_lcov(NAME test-coverage
    EXECUTABLE test-containers
    COMMAND test-device
    COMMAND test-devices
    COMMAND test-json
    COMMAND test-rlp
    COMMAND test-sys
    COMMAND test-usb-common
    COMMAND test-usbh)
    # COMMAND test-network
    # COMMAND binding-network-cpp-test)

endif()

add_subdirectory(src)

if(WITH_CPP_BINDING)
  add_subdirectory(bindings/cpp)
endif()

if(WITH_NODEJS_BINDING)
  message(STATUS "Building NODEJS binding")
  message(STATUS "Make sure to build NodeJS Binding by using npm install command!")
  add_subdirectory(bindings/nodejs)
endif()
