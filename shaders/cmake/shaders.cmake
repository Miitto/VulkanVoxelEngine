find_program(SLANGC_EXECUTABLE NAMES slangc REQUIRED)

function(compile_shader target)
  set(MULTIVALUE SOURCES INCLUDES)
  cmake_parse_arguments(PARSE_ARGV 0 arg "" "" "${MULTIVALUE}")
  set(ENTRY_POINTS -entry vert -entry frag)
  set(OUTPUTS "")
  set(INCLUDED_FILES "")
  message(STATUS "Including ${arg_INCLUDES} in shader compilation")
  foreach(file ${arg_INCLUDES})
    set(INCLUDED_FILES ${INCLUDED_FILES} include/${file}.slang)
  endforeach()
  message(STATUS "Included files: ${INCLUDED_FILES}")
  foreach(source ${arg_SOURCES})
    set(SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${source}.slang)
    set(OUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/build/${source}.spv)
    add_custom_command(
            OUTPUT ${OUT_FILE}
            DEPENDS ${source}.slang ${INCLUDED_FILES}
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
