FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG master)
FetchContent_MakeAvailable(glm)

function(link_glm TARGET_NAME)
  target_link_libraries(${TARGET_NAME} PUBLIC glm::glm)
  target_compile_definitions(${TARGET_NAME} PUBLIC GLM_FORCE_RADIANS GLM_FORCE_DEPTH_ZERO_TO_ONE GLM_FORCE_LEFT_HANDED)
endfunction()
