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

set(sources ./src/binding.cc ./src/NetwJS.cc ./src/LogJS.cc)
set(headers ./src/NetwJS.h ./src/LogJS.h)
set(defs "LINQ_NETWORK_NODEJS_BINDING")

set(libs
  network-static
  network-cpp-binding
  ${LIBCZMQ_STATIC_LIBS}
  ${LIBZMQ_STATIC_LIBS}
  ${CMAKE_JS_LIB}
  uuid
)

if(BUILD_LINQD)
  list(APPEND defs "-DBUILD_LINQD")
  list(APPEND libs 
    database-static
    http-static   
    ${TLS_SHARED_LIBRARIES}
  )
endif()

if(BUILD_USBH)
  list(APPEND defs "-DBUILD_USBH")
  list(APPEND libs usbh-static)
endif()

message(STATUS "libs: ${libs}")

###############################################################################
# Build network-js
###############################################################################
include_directories(${CMAKE_JS_INC})
add_library(network-js SHARED ${sources} ${headers} ${CMAKE_JS_SRC})
set_target_properties(network-js PROPERTIES 
  OUTPUT_NAME "linq"
  PREFIX "" 
  SUFFIX ".node")
target_include_directories(network-js PRIVATE ${CMAKE_JS_INC} ${NODE_ADDON_API_DIR})
target_link_libraries(network-js  ${libs})
target_compile_definitions(network-js PRIVATE ${defs})

install(TARGETS network-js
        LIBRARY DESTINATION . #bindings-js looks in root build folder (not in lib folder)
        ARCHIVE DESTINATION .
        PUBLIC_HEADER DESTINATION include/altronix)
