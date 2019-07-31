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

function(set_libwad_properties target)
  set_target_properties(${target} PROPERTIES
    PREFIX ""
    PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/libwad.h"
    C_VISIBILITY_PRESET hidden
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON)

  target_link_libraries(${target} mbedtls)
  target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/externals/mbedtls/include)

  if (MSVC)
    target_compile_definitions(${target} PRIVATE -D_CRT_SECURE_NO_WARNINGS)
  endif()

  set_target_properties(${target} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

  if (ENABLE_INSTALL)
    install(TARGETS ${target}
            LIBRARY DESTINATION "lib"
            RUNTIME DESTINATION "lib"
            ARCHIVE DESTINATION "bin"
            PUBLIC_HEADER DESTINATION "include")
    if (MSVC)
      install(TARGETS ${target}
              RUNTIME DESTINATION "bin")
    endif()
  endif()
endfunction(set_libwad_properties)

# Version info
configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/version.h PROPERTIES GENERATED TRUE)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if (BUILD_SHARED)
  add_library(libwad SHARED ${SOURCES})
  set_libwad_properties(libwad)
endif()

if (BUILD_STATIC)
  add_library(libwadstatic STATIC ${SOURCES})
  set_libwad_properties(libwadstatic)
endif()

if (ENABLE_TOOLS)
    add_subdirectory(tools)
endif()
