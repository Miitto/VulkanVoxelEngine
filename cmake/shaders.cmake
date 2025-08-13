find_program(SLANGC_EXECUTABLE NAMES slangc REQUIRED)

function(compile_shader target)
  cmake_parse_arguments(PARSE_ARGV 0 arg "" "" "SOURCES")
  set(ENTRY_POINTS -entry vertMain -entry fragMain)
  set(OUTPUTS "")
  foreach(source ${arg_SOURCES})
    set(SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${source}.slang)
    set(OUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/build/${source}.spv)
    add_custom_command(
            OUTPUT ${OUT_FILE}
            DEPENDS ${source}.slang
            COMMAND ${SLANGC_EXECUTABLE} ${SOURCE_FILE} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o ${OUT_FILE}
            COMMENT "Compiling shader ${source} to SPIR-V"
            VERBATIM
        )
    set(OUTPUTS ${OUTPUTS} ${OUT_FILE})
  endforeach()

  add_custom_target(${target} ALL
    DEPENDS ${OUTPUTS}
    COMMENT "Compiling shaders for target ${target}"
  )
endfunction()
