if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR
   CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
   CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options("-std=c++17")

  add_compile_options(-Wall)
  add_compile_options(-Werror)
  add_compile_options(-Wextra)
  add_compile_options(-pedantic)
  add_compile_options(-Wno-shadow)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
   CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
  add_compile_options(-Qunused-arguments)
  add_compile_options(-Wno-deprecated-declarations)
  add_compile_options(-Wno-unknown-argument)
endif()


add_executable(unittests main.cpp)
target_compile_options(unittests PRIVATE ${BASE_COMPILE_FLAGS})
# target_link_libraries(unittests PRIVATE gtest_main matchit)
target_link_libraries(unittests matchit)
gtest_discover_tests(unittests)