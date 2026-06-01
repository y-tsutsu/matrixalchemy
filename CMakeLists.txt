cmake_minimum_required(VERSION 3.22)

project(matrixalchemy
  VERSION 0.1.0
  DESCRIPTION "A small OpenGL playground for learning 3D graphics fundamentals."
  LANGUAGES C CXX)

option(MATRIXALCHEMY_ENABLE_IMGUI "Enable Dear ImGui debug UI when the package is available" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)

find_package(glm CONFIG QUIET)
if(NOT glm_FOUND)
  find_path(GLM_INCLUDE_DIR glm/glm.hpp REQUIRED)
endif()

add_library(matrixalchemy_glad STATIC
  external/glad/src/gl.c)

target_include_directories(matrixalchemy_glad
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include)

add_executable(matrixalchemy
  src/App.cpp
  src/AxisGizmo.cpp
  src/Character.cpp
  src/ColoredMesh.cpp
  src/FileSystem.cpp
  src/GridFloor.cpp
  src/Main.cpp
  src/OrbitCamera.cpp
  src/RotatingCube.cpp
  src/ShaderProgram.cpp)

target_include_directories(matrixalchemy
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(matrixalchemy PRIVATE GLFW_INCLUDE_NONE)
target_compile_definitions(matrixalchemy PRIVATE MATRIXALCHEMY_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

if(glm_FOUND)
  target_link_libraries(matrixalchemy PRIVATE glm::glm)
else()
  target_include_directories(matrixalchemy PRIVATE ${GLM_INCLUDE_DIR})
endif()

target_link_libraries(matrixalchemy
  PRIVATE
    OpenGL::GL
    glfw
    matrixalchemy_glad)

if(MATRIXALCHEMY_ENABLE_IMGUI)
  find_package(imgui CONFIG QUIET)
  if(TARGET imgui::imgui)
    target_sources(matrixalchemy
      PRIVATE
        src/DebugUi.cpp)
    target_link_libraries(matrixalchemy PRIVATE imgui::imgui)
    target_compile_definitions(matrixalchemy PRIVATE MATRIXALCHEMY_HAS_IMGUI=1)
  else()
    pkg_check_modules(IMGUI QUIET IMPORTED_TARGET imgui)
    if(TARGET PkgConfig::IMGUI)
      target_sources(matrixalchemy
        PRIVATE
          src/DebugUi.cpp)
      target_link_libraries(matrixalchemy PRIVATE PkgConfig::IMGUI)
      target_compile_definitions(matrixalchemy PRIVATE MATRIXALCHEMY_HAS_IMGUI=1)
    else()
      message(STATUS "Dear ImGui was not found. Building without the debug UI.")
    endif()
  endif()
endif()

if(MSVC)
  target_compile_options(matrixalchemy PRIVATE /W4 /permissive-)
else()
  target_compile_options(matrixalchemy PRIVATE -Wall -Wextra -Wpedantic)
endif()
