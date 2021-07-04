cmake_minimum_required(VERSION 3.15...3.19)
project(
    Matchit
    VERSION 0.0.1
    LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if (NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
message(STATUS "CXX_STANDARD: ${CMAKE_CXX_STANDARD}")

list(APPEND
    BASE_COMPILE_FLAGS
    "$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wall;-Wextra;-pedantic;-Werror;-Wno-shadow;-Wconversion;-Wsign-conversion>"
        "$<$<CXX_COMPILER_ID:MSVC>:/W4>") # /WX for -Werror

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
    include(CodeCoverage)
    list(APPEND BASE_COMPILE_FLAGS "-g;-O0;-fprofile-arcs;-ftest-coverage")
endif() #CMAKE_BUILD_TYPE STREQUAL "Coverage"

if (CMAKE_BUILD_TYPE STREQUAL "MSAN")
    add_link_options("-L${PROJECT_SOURCE_DIR}/libcxx_msan/lib;-lc++abi")
endif() #CMAKE_BUILD_TYPE STREQUAL "MSAN"

# Target.
add_library(matchit INTERFACE)
target_include_directories(matchit INTERFACE
  ${PROJECT_SOURCE_DIR}/include)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(Sanitizers)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(test)
        add_subdirectory(sample)
    endif()
endif()
