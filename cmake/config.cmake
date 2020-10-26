# Version
set(V_MAJOR 0)
set(V_MINOR 0)
set(V_PATCH 4)
set(V_RC -rc8)
set(LINQ_NETWORK_VERSION "v${V_MAJOR}.${V_MINOR}.${V_PATCH}${V_RC}")
function (append_version_compiler_flags dst)
  list(APPEND arg "${${dst}}")
  list(APPEND arg "LINQ_NETWORK_VERSION=\"${LINQ_NETWORK_VERSION}\"")
  set(${dst} "${arg}" PARENT_SCOPE)
endfunction()

# Library Output options
option(BUILD_SHARED "Whether or not to build the shared object"  ON)
option(BUILD_STATIC "Whether or not to build the static archive" ON)
set(BUILDROOT_DIR "" CACHE STRING "set to fullpath of root buildroot directory outside of buildroot")

# Build (or find) dependencies
option(BUILD_DEPENDENCIES "Build all dependencies (except openssl)" OFF)

# binary output options
option(BUILD_LINQD "Whether or not to build the daemon" OFF)
option(BUILD_USBD "Whether or not to build USB device support" OFF)
option(BUILD_USBH "Whether or not to build USB host support" OFF)
option(BUILD_DEBUG "Build with debug flags?" OFF)
option(BUILD_APPS "Build the applications in apps folder" OFF)

# bindings
option(WITH_NODEJS_BINDING "Build the NODEJS bindings" OFF)
option(WITH_CPP_BINDING "Build the C++ bindings" ON)

# Testing (TODO - set USBD/USBH/LINQD if test suite is enabled)
option(ENABLE_TESTING "Build test suite" OFF)
option(DISABLE_PASSWORD "Disable password for http service" OFF)

# LinQ Log Level Options
list(APPEND LOG_OPTIONS "TRACE" "DEBUG" "INFO" "WARN" "FATAL" "NONE")
set(LOG_LEVEL "INFO" CACHE STRING "set logging level ${LOG_OPTIONS}")
function(append_log_level_compiler_flags dst)
  MESSAGE(STATUS "LOG_LEVEL: [${LOG_LEVEL}]")
  if(${LOG_LEVEL} STREQUAL "TRACE")
    set(LOG_LEVEL_INT 6)
  elseif(${LOG_LEVEL} STREQUAL "DEBUG")
    set(LOG_LEVEL_INT 5)
  elseif(${LOG_LEVEL} STREQUAL "INFO")
    set(LOG_LEVEL_INT 4)
  elseif(${LOG_LEVEL} STREQUAL "WARN")
    set(LOG_LEVEL_INT 3)
  elseif(${LOG_LEVEL} STREQUAL "ERROR")
    set(LOG_LEVEL_INT 2)
  elseif(${LOG_LEVEL} STREQUAL "FATAL")
    set(LOG_LEVEL_INT 1)
  else()
    set(LOG_LEVEL_INT 0)
  endif()
  list(APPEND arg "${${dst}}")
  list(APPEND arg "-DLINQ_LOG_LEVEL=${LOG_LEVEL_INT}")
  set(${dst} "${arg}" PARENT_SCOPE)
endfunction()
