add_library(engine_core)

add_subdirectory(src)

target_sources(PUBLIC
  FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
)

include(imgui)
include(spdlog)
include(glm)

link_imgui(engine_core PUBLIC)
link_spdlog(engine_core PUBLIC)
link_glm(engine_core PUBLIC)

set(WINDOW_LIB_OPTIONS GLFW SDL)
set(WINDOW_LIB GLFW CACHE STRING "Window library to use")
set_property(CACHE WINDOW_LIB PROPERTY STRINGS ${WINDOW_LIB_OPTIONS})

set(HAS_VALID_WINDOW_LIB FALSE)
if(WINDOW_LIB STREQUAL "GLFW")
  set(HAS_VALID_WINDOW_LIB TRUE)
  include(glfw)
  link_glfw(engine_core PUBLIC)
  target_compile_definitions(engine_core PUBLIC USE_GLFW)
elseif(WINDOW_LIB STREQUAL "SDL")
  set(HAS_VALID_WINDOW_LIB TRUE)
  include(sdl)
  link_sdl(engine_core PUBLIC)
  target_compile_definitions(engine_core PUBLIC USE_SDL)
endif()

if(NOT HAS_VALID_WINDOW_LIB)
  message(FATAL_ERROR
    "Invalid WINDOW_LIB option: ${WINDOW_LIB}.
    Valid options are: ${WINDOW_LIB_OPTIONS}"
  )
endif()
