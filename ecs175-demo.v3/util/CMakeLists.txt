# some extra helper functions used by TA only
unset(extra_sources)
if(OFFSCREEN_RENDERING)
  list(APPEND extra_sources hidden.cpp)
endif()

# here we are building one target for each OpenGL version
foreach(suffix compatible core-3.3 core-4.1)

  # create the library
  add_library(util-${suffix}
    util.cpp ${extra_sources})

  # add library dependencies
  target_link_libraries(util-${suffix} PUBLIC
    glad-${suffix} imgui-${suffix} ${gl_libs})

  # setup include directries for this target (modern cmake)
  target_include_directories(util-${suffix} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)

  # similarly, we can setup c++ definitions
  target_compile_definitions(util-${suffix} PUBLIC 
    $<IF:$<BOOL:${OFFSCREEN_RENDERING}>,OFFSCREEN_RENDERING=1,OFFSCREEN_RENDERING=0>)

  # and other properties
  set_target_properties(util-${suffix} PROPERTIES CXX_STANDARD 11)

endforeach()
