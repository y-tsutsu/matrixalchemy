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
find_package(PkgConfig QUIET)

find_package(glm CONFIG QUIET)
if(NOT glm_FOUND)
  find_path(GLM_INCLUDE_DIR glm/glm.hpp REQUIRED)
endif()

find_path(STB_INCLUDE_DIR stb_image.h PATH_SUFFIXES stb REQUIRED)

set(MATRIXALCHEMY_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(MATRIXALCHEMY_SHADER_SOURCES_HPP
  ${MATRIXALCHEMY_GENERATED_INCLUDE_DIR}/matrixalchemy/render/ShaderSources.hpp)

add_custom_command(
  OUTPUT ${MATRIXALCHEMY_SHADER_SOURCES_HPP}
  COMMAND ${CMAKE_COMMAND}
    -DVERTEX_SHADER=${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/color.vert
    -DFRAGMENT_SHADER=${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/color.frag
    -DOUTPUT_FILE=${MATRIXALCHEMY_SHADER_SOURCES_HPP}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateShaderSources.cmake
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/color.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/color.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateShaderSources.cmake
  VERBATIM)

add_library(matrixalchemy_glad STATIC
  external/glad/src/gl.c)

target_include_directories(matrixalchemy_glad
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include)

add_executable(matrixalchemy
  src/app/App.cpp
  src/asset/Cgltf.cpp
  src/asset/Model.cpp
  src/main.cpp
  src/platform/FileSystem.cpp
  src/render/ColoredMesh.cpp
  src/render/ModelMesh.cpp
  src/render/ShaderProgram.cpp
  src/render/Shadow.cpp
  src/render/Texture2D.cpp
  src/scene/AxisGizmo.cpp
  src/scene/Character.cpp
  src/scene/GridFloor.cpp
  src/scene/OrbitCamera.cpp
  src/scene/RotatingCube.cpp
  ${MATRIXALCHEMY_SHADER_SOURCES_HPP})

target_include_directories(matrixalchemy
  PRIVATE
    ${MATRIXALCHEMY_GENERATED_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/cgltf
    ${STB_INCLUDE_DIR})

target_compile_definitions(matrixalchemy PRIVATE GLFW_INCLUDE_NONE)
target_compile_definitions(matrixalchemy PRIVATE MATRIXALCHEMY_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_custom_command(TARGET matrixalchemy POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/models/saurus.vrm
    $<TARGET_FILE_DIR:matrixalchemy>/saurus.vrm
  VERBATIM)

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
        src/ui/DebugUi.cpp)
    target_link_libraries(matrixalchemy PRIVATE imgui::imgui)
    target_compile_definitions(matrixalchemy PRIVATE MATRIXALCHEMY_HAS_IMGUI=1)
  else()
    if(PkgConfig_FOUND)
      pkg_check_modules(IMGUI QUIET IMPORTED_TARGET imgui)
    endif()
    if(TARGET PkgConfig::IMGUI)
      target_sources(matrixalchemy
        PRIVATE
          src/ui/DebugUi.cpp)
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
