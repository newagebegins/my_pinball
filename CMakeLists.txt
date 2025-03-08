cmake_minimum_required(VERSION 3.31)

project(my_pinball)

find_package(OpenGL REQUIRED)

include(FetchContent)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.4
  GIT_SHALLOW    ON
  GIT_PROGRESS   ON
)
FetchContent_MakeAvailable(glfw)

add_executable(my_pinball main.cpp)
target_compile_options(my_pinball PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>)
target_link_libraries(my_pinball PRIVATE OpenGL::GL glfw)
