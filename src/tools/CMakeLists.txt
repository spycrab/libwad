## Tools

function(set_util_properties target)
  if (BUILD_STATIC)
    target_link_libraries(${target} libwadstatic)
  else()
    target_link_libraries(${target} libwad)
  endif()

  if (ENABLE_INSTALL)
    install(TARGETS ${target} DESTINATION bin)
  endif()

  if (MSVC)
    target_compile_definitions(${target} PRIVATE -D_CRT_SECURE_NO_WARNINGS)
  endif()

  target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/externals/optparse)
endfunction(set_util_properties)

add_executable(wadinfo wadinfo.c info.h info.c)
add_executable(tmdinfo tmdinfo.c info.h info.c)
add_executable(certinfo certinfo.c info.h info.c)
add_executable(ticketinfo ticketinfo.c info.h info.c)
add_executable(wadextract wadextract.c)
add_executable(wadverify wadverify.c)
add_executable(wadglue wadglue.c)

set_util_properties(wadinfo)
set_util_properties(tmdinfo)
set_util_properties(certinfo)
set_util_properties(ticketinfo)
set_util_properties(wadextract)
set_util_properties(wadverify)
set_util_properties(wadglue)
