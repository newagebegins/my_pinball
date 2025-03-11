cmake_minimum_required(VERSION 3.31)

project(my_pinball)

find_package(OpenGL REQUIRED)

include(FetchContent)
FetchContent_Declare(
  glfw
  URL https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip
)
FetchContent_Declare(
  glm
  URL https://github.com/g-truc/glm/archive/refs/tags/1.0.1.zip
)
FetchContent_MakeAvailable(glfw glm)

add_subdirectory(deps/glad)

add_executable(my_pinball main.cpp)
target_compile_features(my_pinball PRIVATE cxx_std_17)
target_compile_options(my_pinball PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror -pedantic-errors -Wall -Wextra -Wconversion -Wsign-conversion>
)
target_link_libraries(my_pinball PRIVATE OpenGL::GL glfw glad glm::glm)
