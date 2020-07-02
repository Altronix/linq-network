set(CMAKE_POSITION_INDEPENDENT_CODE on)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

###############################################################################
# node-addon-api
###############################################################################
execute_process(COMMAND node -p "require('node-addon-api').include"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE NODE_ADDON_API_DIR)
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

set(sources ./src/binding.cc ./src/linq_network_js.cc)
set(headers ./src/linq_network_js.h)
set(defs "LINQ_NETWORK_NODEJS_BINDING")

if(BUILD_LINQD)
  list(APPEND sources ./src/linq_daemon_js.cc)
  list(APPEND headers ./src/linq_daemon_js.h)
  list(APPEND defs "BUILD_LINQD")
  set(libs
    linq-daemon-static
    linq-database-static
    linq-http-static
    linq-network-static
    linq-network-cpp-binding
    linq-utils-static
    ${JSMN_WEB_TOKENS_STATIC_LIBS}
    ${LIBCZMQ_STATIC_LIBS}
    ${LIBZMQ_STATIC_LIBS}
    ${CMAKE_JS_LIB}
    ${TLS_SHARED_LIBRARIES}
    uuid
    )
else()
  set(libs
    linq-network-static
    linq-network-cpp-binding
    linq-utils-static
    ${LIBCZMQ_STATIC_LIBS}
    ${LIBZMQ_STATIC_LIBS}
    ${CMAKE_JS_LIB}
    uuid)
endif()

###############################################################################
# Build linq-network-js
###############################################################################
include_directories(${CMAKE_JS_INC})
add_library(linq-network-js SHARED ${sources} ${headers} ${CMAKE_JS_SRC})
set_target_properties(linq-network-js PROPERTIES 
  OUTPUT_NAME "linq-network"
  PREFIX "" 
  SUFFIX ".node")
target_include_directories(linq-network-js PRIVATE ${CMAKE_JS_INC} ${NODE_ADDON_API_DIR})
target_link_libraries(linq-network-js  ${libs})
target_compile_definitions(linq-network-js PRIVATE ${defs})

install(TARGETS linq-network-js
        LIBRARY DESTINATION . #bindings-js looks in root build folder (not in lib folder)
        ARCHIVE DESTINATION .
        PUBLIC_HEADER DESTINATION include/altronix)
