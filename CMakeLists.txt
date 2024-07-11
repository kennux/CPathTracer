cmake_minimum_required(VERSION 3.25)
set(CMAKE_VERBOSE_MAKEFILE ON)
project(CPathTracer LANGUAGES C)

set(CMAKE_C_STANDARD 23)

file(GLOB pathtracer_SRC
    "pathtracer/*.h"
    "pathtracer/*.c"
)

add_executable(CPathTracer ${pathtracer_SRC})

# OpenGL GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)
target_link_libraries(CPathTracer glfw)
find_package(OpenGL REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS} )
target_link_libraries(CPathTracer OpenGL::GL)

include(CMakePrintHelpers)
if (CMAKE_C_COMPILER MATCHES "clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -march=native")
endif ()