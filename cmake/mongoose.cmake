set(source ${CMAKE_SOURCE_DIR}/external/mongoose/mongoose.c)
set(header ${CMAKE_SOURCE_DIR}/external/mongoose/mongoose.h)
add_library(mongoose STATIC ${source} ${header})
set_target_properties(mongoose 
        PROPERTIES PUBLIC_HEADER "${header}")
install(TARGETS mongoose
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/mongoose)
