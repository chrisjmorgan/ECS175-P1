# welcome to ECS 175
cmake_minimum_required(VERSION 3.0)
project(ecs175-template)

# enable rpath on macos (advanced configuration)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_MACOSX_RPATH 1)
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
option(OFFSCREEN_RENDERING "Enable offline rendering (only for TA)" OFF)

# --------------------------------------
# load/build required libraries
#
# ---> OpenGL (advanced configuration)
# reference https://cmake.org/cmake/help/v3.18/module/FindOpenGL.html
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
# correctly select OpenGL targets (modern CMake)
if(TARGET OpenGL::OpenGL)
  list(APPEND gl_libs OpenGL::OpenGL)
else()
  list(APPEND gl_libs OpenGL::GL)
endif()
if(TARGET OpenGL::GLU)
  list(APPEND gl_libs OpenGL::GLU)
endif()

# ---> build glfw from source
if (NOT OFFSCREEN_RENDERING)
  # use GLFW for interactive GL rendering 
  # as it is cross platform compatable
  if(TARGET OpenGL::GLX)
    list(APPEND gl_libs OpenGL::GLX)
  endif()
  set(GLFW_USE_OSMESA OFF)
else()
  # enable software rendering via OSMesa
  set(GLFW_USE_OSMESA ON)
endif()
# configure other options
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(${PROJECT_SOURCE_DIR}/glfw-3.3.2)
# hide these options by default (try the GUI interface :D)
mark_as_advanced(GLFW_INSTALL 
  GLFW_BUILD_DOCS GLFW_BUILD_TESTS GLFW_BUILD_EXAMPLES
  GLFW_USE_OSMESA GLFW_USE_WAYLAND GLFW_VULKAN_STATIC)
list(APPEND gl_libs glfw)

# ---> build glad from source
add_subdirectory(glad-compatible)
add_subdirectory(glad-core-3.3)
add_subdirectory(glad-core-4.1)
set(ALL_GL_VERSIONS compatible core-3.3 core-4.1)

# ---> use the bin2c tool to embed shader
add_executable(bin2c bin2c/bin2c.c) # compile bin2c obviously
function(target_add_embeded_shaders target)
  # how to parse keyword arguments in a cmake function
  set(options)
  set(oneArgs OUTPUT_NAME)
  set(mulArgs SHADERS)
  cmake_parse_arguments(in "${options}" "${oneArgs}" "${mulArgs}" ${ARGN})
  if(in_UNPARSED_ARGUMENTS)
    foreach(arg ${in_UNPARSED_ARGUMENTS})
      message(WARNING "Unparsed argument: ${arg}")
    endforeach()
  endif()
  # compute a command for bin2c
  unset(all_shaders)
  set(the_command bin2c -o ${in_OUTPUT_NAME})
  foreach(s ${in_SHADERS})
    # the shader is provided in the form of 'name=filename'
    if("${s}" MATCHES "^([^=]*)=([^=]*)$")
      list(APPEND all_shaders ${CMAKE_MATCH_2})
      set(the_command ${the_command} -n ${CMAKE_MATCH_1} ${CMAKE_MATCH_2})
    endif()
    # the shader is provided in the form of 'filename'
    if("${s}" MATCHES "^([^=]*)$")
      list(APPEND all_shaders ${CMAKE_MATCH_1})
      set(the_command ${the_command} ${CMAKE_MATCH_1})
    endif()
  endforeach()
  # now generate the file
  string(REPLACE "/" "_" output_identifier ${in_OUTPUT_NAME})
  add_custom_target(_embeded_shaders_${target}_${output_identifier}
    COMMAND ${the_command}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    COMMENT "generate embeded shaders for ${target}"
    SOURCES ${all_shaders})
  add_dependencies(${target} _embeded_shaders_${target}_${output_identifier})
endfunction()

# ---> build "Dear ImGui"
add_subdirectory(imgui)

# ---> build the project utility library
add_subdirectory(util)

# build sub-projects
#add_subdirectory(demo)
add_subdirectory(projects/p1_skeleton)
