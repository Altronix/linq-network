#--------------------------------------------------------------------------
# Lib of mock functions for test
#--------------------------------------------------------------------------
add_library(mock-czmq mock_zmsg.c mock_zsock.c)
target_link_libraries(mock-czmq czmq zmq)
target_include_directories(mock-czmq PUBLIC .)
