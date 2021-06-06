include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG dcc92d0ab6c4ce022162a23566d44f673251eee4)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR}
                    EXCLUDE_FROM_ALL)
endif()

message(STATUS "GTest binaries are present at ${googletest_BINARY_DIR}")