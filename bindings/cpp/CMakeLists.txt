set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(atx-net-cpp-binding INTERFACE)
target_include_directories(atx-net-cpp-binding INTERFACE ./)
# set_target_properties(atx-net-cpp-binding PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR})
install(TARGETS atx-net-cpp-binding
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include/altronix)
