# test-fixture

set(linq_common_SOURCE_DIR ${EXTERNAL_DIR}/linq-common)

### build linq-common ###
add_definitions(-DCONFIG_LINUX_EMU)
add_definitions(-DUSYS_CONFIG_UNIX)
add_definitions(-DUZMTP_USE_TLS)
add_subdirectory(${linq_common_SOURCE_DIR}/libcrypt/wolfssl)
add_subdirectory(${linq_common_SOURCE_DIR}/libosal)
add_subdirectory(${linq_common_SOURCE_DIR}/libuzmtp)
add_subdirectory(${linq_common_SOURCE_DIR}/libitem)
add_subdirectory(${linq_common_SOURCE_DIR}/libatx)
add_subdirectory(${linq_common_SOURCE_DIR}/test/mocks)
target_include_directories(atx PUBLIC ${linq_common_SOURCE_DIR}/libatx)
