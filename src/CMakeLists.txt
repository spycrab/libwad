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

# Version info
configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/version.h PROPERTIES GENERATED TRUE)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if (BUILD_SHARED)
  add_library(libwad SHARED ${SOURCES})

  set_target_properties(libwad PROPERTIES PREFIX "")
  set_target_properties(libwad PROPERTIES PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/libwad.h")
  set_target_properties(libwad PROPERTIES C_VISIBILITY_PRESET hidden) 

  target_link_libraries(libwad mbedtls)
  target_include_directories(libwad PRIVATE ${CMAKE_SOURCE_DIR}/externals/mbedtls/include)

  if (ENABLE_INSTALL)
    install(TARGETS libwad
            LIBRARY DESTINATION "lib"
            RUNTIME DESTINATION "lib"
            ARCHIVE DESTINATION "bin"
            PUBLIC_HEADER DESTINATION "include")
    if (MSVC)
      install(TARGETS libwad
              RUNTIME DESTINATION "bin")
    endif()
  endif()
endif()

if (BUILD_STATIC)
  add_library(libwadstatic STATIC ${SOURCES})

  set_target_properties(libwadstatic PROPERTIES PREFIX "")
  set_target_properties(libwadstatic PROPERTIES PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/libwad.h")
  set_target_properties(libwadstatic PROPERTIES C_VISIBILITY_PRESET hidden) 

  target_link_libraries(libwadstatic mbedtls)
  target_include_directories(libwadstatic PRIVATE ${CMAKE_SOURCE_DIR}/externals/mbedtls/include)

  if (ENABLE_INSTALL)
    install(TARGETS libwadstatic
            LIBRARY DESTINATION "lib"
            ARCHIVE DESTINATION "bin"
            PUBLIC_HEADER DESTINATION "include")
   endif()
endif()
		
if (ENABLE_TOOLS)
    add_subdirectory(tools)
endif()
