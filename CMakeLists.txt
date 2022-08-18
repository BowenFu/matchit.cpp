cmake_minimum_required(VERSION 3.15...3.19)

project(
    "matchit"
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION
    "match(it): A lightweight single-header pattern-matching library for C++17 with macro-free APIs."
    HOMEPAGE_URL  "https://github.com/BowenFu/matchit.cpp")

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

include(GNUInstallDirs)

# Target.
add_library(matchit INTERFACE)

target_include_directories(
  matchit
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_features(matchit INTERFACE cxx_std_17)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(Sanitizers)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(test)
        add_subdirectory(sample)
    endif()
endif()


install(
    TARGETS matchit
    EXPORT matchitTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
        DESTINATION include)

install(
    EXPORT matchitTargets
    FILE matchitTargets.cmake
    NAMESPACE matchit::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/matchit)

configure_file(${PROJECT_SOURCE_DIR}/cmake/matchitConfig.cmake.in
                matchitConfig.cmake @ONLY)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    matchitConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/matchitConfig.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/matchitConfigVersion.cmake
         DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/matchit)

install(
    FILES ${PROJECT_SOURCE_DIR}/LICENSE
    DESTINATION ${CMAKE_INSTALL_DOCDIR})
