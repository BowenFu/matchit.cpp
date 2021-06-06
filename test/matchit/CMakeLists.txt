add_executable(unittests main.cpp)
target_compile_options(unittests PRIVATE ${BASE_COMPILE_FLAGS})
target_link_libraries(unittests PRIVATE matchit)
set_target_properties(unittests PROPERTIES CXX_EXTENSIONS OFF)