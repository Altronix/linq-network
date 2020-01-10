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
# Build linq-netw-js
###############################################################################
add_library(linq-netw-js SHARED 
  ./src/binding.cc
  ./src/linqnetwork.h
  ./src/linqnetwork.cc)
set_target_properties(linq-netw-js PROPERTIES 
  PREFIX "" 
  SUFFIX ".node")
target_include_directories(linq-netw-js PRIVATE ${CMAKE_JS_INC} ${NODE_ADDON_API_DIR})
target_link_libraries(linq-netw-js 
  linq-netw-static
  linq-netw-cpp-binding
  czmq
  zmq
  ${CMAKE_JS_LIB}
  uuid
  )

install(TARGETS linq-netw-js
        LIBRARY DESTINATION . #bindings-js looks in root build folder (not in lib folder)
        ARCHIVE DESTINATION .
        PUBLIC_HEADER DESTINATION include/altronix)
