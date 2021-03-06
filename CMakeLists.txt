cmake_minimum_required(VERSION 3.1)
project(libwad C)

option(ENABLE_TOOLS "Build tools" ON)
option(ENABLE_DOCS "Build docs" ON)
option(ENABLE_INSTALL "Install library and tools" ON)
option(BUILD_SHARED "Build shared library" ON)
option(BUILD_STATIC "Build static library" OFF)

if (NOT (BUILD_SHARED OR BUILD_STATIC))
  message(FATAL_ERROR "BUILD_SHARED (and) or BUILD_STATIC need to be enabled!")
endif()

include_directories(include)

# mbedtls
set(ENABLE_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(INSTALL_MBEDTLS_HEADERS OFF CACHE BOOL "" FORCE)
set(USE_SHARED_MBEDTLS_LIBRARY OFF CACHE BOOL "" FORCE)
set(USE_STATIC_MBEDTLS_LIBRARY ON CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/externals/mbedtls)

# We need -fPIC for static linking with our shared library
set_target_properties(mbedtls PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(mbedcrypto PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Generate version information

find_package(Git)
if(GIT_FOUND)
  execute_process(
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMAND ${GIT_EXECUTABLE} status
    RESULT_VARIABLE IS_GIT_REPO
    OUTPUT_QUIET)

  if (IS_GIT_REPO EQUAL 0)
    # Adadpted from Dolphin
    # Make sure version information gets re-run when the current Git HEAD changes
    execute_process(
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      COMMAND ${GIT_EXECUTABLE} rev-parse --git-path HEAD
      OUTPUT_VARIABLE LIBWAD_GIT_HEAD
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${LIBWAD_GIT_HEAD}")

    execute_process(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} rev-parse --symbolic-full-name HEAD
      OUTPUT_VARIABLE LIBWAD_GIT_HEAD_SYMBOLIC
      OUTPUT_STRIP_TRAILING_WHITESPACE)

        execute_process(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      COMMAND ${GIT_EXECUTABLE} rev-parse --git-path ${LIBWAD_GIT_HEAD_SYMBOLIC}
      OUTPUT_VARIABLE LIBWAD_GIT_HEAD_SYMBOLIC_FILENAME
      OUTPUT_STRIP_TRAILING_WHITESPACE)

        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${LIBWAD_GIT_HEAD_SYMBOLIC_FILENAME}")

    execute_process(
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      COMMAND ${GIT_EXECUTABLE} describe --abbrev=0 --tags
      OUTPUT_VARIABLE LIBWAD_LATEST_TAG
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET)

    if (LIBWAD_LATEST_TAG STREQUAL "")
      set(LIBWAD_VERSION "unknown")
    else()
      set(LIBWAD_VERSION "${LIBWAD_LATEST_TAG}")
      set(GIT_REV_OPTS ^${LIBWAD_LATEST_TAG})
    endif()

    execute_process(
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      COMMAND ${GIT_EXECUTABLE} rev-list HEAD ${GIT_REV_OPTS} --count
      OUTPUT_VARIABLE LIBWAD_BUILD_NUMBER
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
      OUTPUT_VARIABLE LIBWAD_BRANCH
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      COMMAND ${GIT_EXECUTABLE} diff --shortstat HEAD
      OUTPUT_VARIABLE IS_GIT_DIRTY
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    if (NOT LIBWAD_BRANCH STREQUAL "master" AND NOT LIBWAD_BRANCH STREQUAL "stable")
      set(LIBWAD_VERSION "${LIBWAD_VERSION}-${LIBWAD_BRANCH}")
    endif()

    if (LIBWAD_BUILD_NUMBER GREATER 0)
      set(LIBWAD_VERSION "${LIBWAD_VERSION}-${LIBWAD_BUILD_NUMBER}")
    endif()

    if (NOT IS_GIT_DIRTY STREQUAL "")
      set(LIBWAD_VERSION "${LIBWAD_VERSION}-dirty")
    endif()
  endif()

else()
   # Fallback if git is unavailable
   set(LIBWAD_VERSION "0.1.5")
endif()

add_subdirectory(src)

## Docs
if (ENABLE_DOCS)
    find_package(Doxygen)

    if(DOXYGEN_FOUND)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        add_custom_target(docs ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs/html DESTINATION share/doc/libwad)
        endif()
endif()
