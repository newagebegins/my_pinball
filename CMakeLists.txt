cmake_minimum_required(VERSION 3.31)

project(my_pinball)

find_package(OpenGL REQUIRED)

include(FetchContent)
FetchContent_Declare(
  glfw
  URL https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip
)
FetchContent_MakeAvailable(glfw)

add_executable(my_pinball main.cpp)
target_compile_options(my_pinball PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>)
target_link_libraries(my_pinball PRIVATE OpenGL::GL glfw)
