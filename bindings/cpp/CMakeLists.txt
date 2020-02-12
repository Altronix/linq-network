set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(linq-network-cpp-binding INTERFACE)
target_include_directories(linq-network-cpp-binding INTERFACE ./)
# set_target_properties(linq-network-cpp-binding PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR})
install(TARGETS linq-network-cpp-binding
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include/altronix)
