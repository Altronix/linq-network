# Library Output options
option(BUILD_SHARED "Whether or not to build the shared object"  ON)
option(BUILD_STATIC "Whether or not to build the static archive" ON)

# Library dependency Options
option(USE_SYSTEM_ZMQ "Look for libzmq and libczmq on target system." OFF)
option(USE_SYSTEM_OPENSSL "Look for openssl on target system." ON)
option(USE_SYSTEM_JSMN_WEB_TOKENS "Look for jwt library on target system" OFF)

# binary output options
option(BUILD_LINQD "Whether or not to build the daemon" ON)

# bindings
option(WITH_NODEJS_BINDING "Build the NODEJS bindings" OFF)
option(WITH_CPP_BINDING "Build the C++ bindings" ON)

# Testing
option(ENABLE_TESTING "Build test suite" OFF)

# LinQ Log Level Options
list(APPEND LOG_OPTIONS "TRACE" "DEBUG" "INFO" "WARN" "FATAL" "NONE")
set(LOG_LEVEL "INFO" CACHE STRING "set logging level ${LOG_OPTIONS}")
function(append_log_level_compiler_flags dst)
  MESSAGE(STATUS "LOG_LEVEL set ${LOG_LEVEL}")
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
  list(APPEND definitions "-DLINQ_LOG_LEVEL=${LOG_LEVEL_INT}")
  set(${dst} ${definitions} PARENT_SCOPE)
endfunction()