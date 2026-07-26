cmake_minimum_required(VERSION 3.25)
include(FetchContent)

project(cnbt_test C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} $ENV{CFLAGS} -Wall -Wextra -Wpedantic -Wno-psabi")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -O0 -g2 -ggdb -fsanitize=address,leak")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3")

file(GLOB CNBT_TEST_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

message(STATUS "cnbt: Fetching munit contents...")
FetchContent_Declare(
  munit
  GIT_REPOSITORY https://github.com/nemequ/munit.git
  GIT_TAG fbbdf14
)
FetchContent_MakeAvailable(munit)

add_executable(${PROJECT_NAME} ${CNBT_TEST_SRC} ${munit_SOURCE_DIR}/munit.c)
target_include_directories(${PROJECT_NAME}
  PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/"
  "${munit_SOURCE_DIR}"
)
target_link_libraries(${PROJECT_NAME} PRIVATE cnbt)
enable_testing()
