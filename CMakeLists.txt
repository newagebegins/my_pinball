cmake_minimum_required(VERSION 3.31)

project(my_pinball)

find_package(OpenGL REQUIRED)

include(FetchContent)
FetchContent_Declare(
  glfw
  URL https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip
)
FetchContent_MakeAvailable(glfw)

add_subdirectory(deps/glad)
add_subdirectory(deps/glm)

add_executable(my_pinball main.cpp game.cpp)
target_compile_features(my_pinball PRIVATE
  # C++17 is needed for inline static variables in classes (like shader code in DefaultShader)
  cxx_std_17
)
target_compile_options(my_pinball PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror -pedantic-errors -Wall -Wextra -Wconversion -Wsign-conversion>
)
target_link_libraries(my_pinball PRIVATE OpenGL::GL glfw glad glm)
