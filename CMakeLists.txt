cmake_minimum_required(VERSION 3.25)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CNBT_VER_MAJ 0)
set(CNBT_VER_MIN 0)
set(CNBT_VER_REV 1)

set(CNBT_VER "${CNBT_VER_MAJ}.${CNBT_VER_MIN}.${CNBT_VER_REV}")

option(CNBT_SHARED_BUILD "Shared object build" OFF)
#option(CNBT_BUILD_TESTS "Build tests" OFF)

set(CNBT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CNBT_SOURCE_DIR "${CNBT_DIR}/src")
set(CNBT_INCLUDE_DIR "${CNBT_DIR}/include")

if (CNBT_SHARED_BUILD)
  set(CNBT_BUILD_TYPE SHARED)
else()
  set(CNBT_BUILD_TYPE STATIC)
endif()

message(STATUS "cnbt: Setting up version ${CNBT_VER_STR}")
project(chimatools VERSION ${CNBT_VER} LANGUAGES C)

file(GLOB_RECURSE CNBT_SOURCE_FILES "${CNBT_SOURCE_DIR}/*.c")
set(CNBT_INCLUDE_FILES "${CNBT_INCLUDE_DIR}/cnbt/cnbt.h")

add_library(${PROJECT_NAME} ${CNBT_BUILD_TYPE})
target_sources(${PROJECT_NAME} PRIVATE ${CNBT_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} -lm)

if (CNBT_SHARED_BUILD)
  target_compile_definitions(${PROJECT_NAME} PRIVATE -DCNBT_SHARED_BUILD)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")

target_include_directories(${PROJECT_NAME} PUBLIC
                           "$<BUILD_INTERFACE:${CNBT_INCLUDE_DIR}>"
                           "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-config
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(EXPORT ${PROJECT_NAME}-config
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/${PROJECT_NAME}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

#if (CNBT_BUILD_EXAMPLES)
#  message(STATUS "cnbt: Building tests")
#  add_subdirectory("${CNBT_DIR}/test")
#endif()
