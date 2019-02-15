include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES 
    ${CMAKE_SOURCE_DIR}/include/libwad.h
    certchain.h
    certchain.c
    data.c
    tmd.h
    tmd.c
    ticket.h
    ticket.c
    util.h
    util.c
    wad.h
    wad.c
)

add_library(libwad ${SOURCES})

set_target_properties(libwad PROPERTIES PREFIX "")
set_target_properties(libwad PROPERTIES PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/libwad.h")

# Version info
configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/version.h PROPERTIES GENERATED TRUE)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(libwad mbedtls)
target_include_directories(libwad PRIVATE ${CMAKE_SOURCE_DIR}/externals/mbedtls/include)

install(TARGETS libwad 
        LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        PUBLIC_HEADER DESTINATION "include")
		
if (ENABLE_TOOLS)
    add_subdirectory(tools)
endif()
