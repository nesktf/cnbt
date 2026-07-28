cmake_minimum_required(VERSION 3.25)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CNBT_VER_MAJ 0)
set(CNBT_VER_MIN 0)
set(CNBT_VER_REV 1)

set(CNBT_VER "${CNBT_VER_MAJ}.${CNBT_VER_MIN}.${CNBT_VER_REV}")

option(CNBT_SHARED_BUILD "Shared object build" OFF)
option(CNBT_BUILD_TESTS "Build tests" OFF)
option(CNBT_ENABLE_ZLIB "Build zlib stream functions" ON)

set(CNBT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CNBT_SOURCE_DIR "${CNBT_DIR}/src")
set(CNBT_INCLUDE_DIR "${CNBT_DIR}/include")

if (CNBT_SHARED_BUILD)
  set(CNBT_BUILD_TYPE SHARED)
else()
  set(CNBT_BUILD_TYPE STATIC)
endif()

message(STATUS "cnbt: Setting up version ${CNBT_VER_STR}")
project(cnbt VERSION ${CNBT_VER} LANGUAGES C)

set(CNBT_SOURCES
  "${CNBT_SOURCE_DIR}/tag.c"
  "${CNBT_SOURCE_DIR}/list.c"
  "${CNBT_SOURCE_DIR}/io.c"
)
set(CNBT_SOURCES_ZLIB
  "${CNBT_SOURCE_DIR}/zlib_io.c"
)
set(CNBT_HEADERS "${CNBT_INCLUDE_DIR}/cnbt/cnbt.h")
list(APPEND CNBT_INCLUDE ${CNBT_HEADERS})

if(CNBT_ENABLE_ZLIB)
  find_package(ZLIB REQUIRED)
  list(APPEND CNBT_LINK ${ZLIB_LIBRARY})
  list(APPEND CNBT_INCLUDE ${ZLIB_INCLUDE_DIRS})
  list(APPEND CNBT_SOURCES ${CNBT_SOURCES_ZLIB})
  list(APPEND CNBT_DEFS -DCNBT_HAS_ZLIB)
endif()

add_library(${PROJECT_NAME} ${CNBT_BUILD_TYPE})
target_sources(${PROJECT_NAME} PRIVATE ${CNBT_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CNBT_INCLUDE})
target_link_libraries(${PROJECT_NAME} ${CNBT_LINK})

if (CNBT_SHARED_BUILD)
  list(APPEND CNBT_DEFS -DCNBT_SHARED_BUILD)
endif()
target_compile_definitions(${PROJECT_NAME} PUBLIC ${CNBT_DEFS})

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")
set_property(TARGET cnbt PROPERTY C_STANDARD 11)

if (CNBT_BUILD_TESTS)
  message(STATUS "cnbt: Building tests")
  add_subdirectory("${CNBT_DIR}/test")
endif()
