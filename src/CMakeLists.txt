set(sources 
	linq.c
	device.c)
set(headers 
	../include/altronix/linq.h
	linq_internal.h
	device.h)

set(linq-c-sources ${sources})
set(linq-c-headers ${headers})
set(linq-c-sources-test ${sources})
set(linq-c-headers-test ${headers})



add_library(linq-c ${linq-c-sources} ${linq-c-headers})
target_link_libraries(linq-c ${deps-libs})
target_include_directories(linq-c PUBLIC ../include)

add_library(linq-c-test ${linq-c-sources-test} ${linq-c-headers-test})
target_link_libraries(linq-c-test ${deps-libs})
target_include_directories(linq-c-test PUBLIC ../include)
