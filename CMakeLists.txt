cmake_minimum_required(VERSION 3.31)

project(my_pinball)

add_executable(my_pinball main.cpp)

target_compile_features(my_pinball PRIVATE cxx_std_20)
target_compile_options(my_pinball PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>)

