set(CMAKE_POSITION_INDEPENDENT_CODE on)

###############################################################################
# node-addon-api
###############################################################################
execute_process(COMMAND node -p "require('node-addon-api').include"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE NODE_ADDON_API_DIR)
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

###############################################################################
# Build atx-net-js
###############################################################################
add_library(atx-net-js SHARED 
  ./src/binding.cc
  ./src/atxnet.h
  ./src/atxnet.cc)
set_target_properties(atx-net-js PROPERTIES 
  PREFIX "" 
  SUFFIX ".node")
target_include_directories(atx-net-js PRIVATE ${CMAKE_JS_INC} ${NODE_ADDON_API_DIR})
target_link_libraries(atx-net-js 
  atx-net-static
  atx-net-cpp-binding
  ${LIBCZMQ_STATIC_LIBS}
  ${LIBZMQ_STATIC_LIBS}
  ${CMAKE_JS_LIB}
  uuid
  )

install(TARGETS atx-net-js
        LIBRARY DESTINATION . #bindings-js looks in root build folder (not in lib folder)
        ARCHIVE DESTINATION .
        PUBLIC_HEADER DESTINATION include/altronix)
