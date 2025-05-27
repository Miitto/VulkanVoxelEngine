target_sources(engine PRIVATE
  main.cpp
  vertex.cpp
)

target_sources(engine PUBLIC FILE_SET modules BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} TYPE CXX_MODULES
  FILES
    app/app.ixx
    app/common.ixx
    app/setup.ixx
    app/class.ixx
)

set_target_properties(engine
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/out/Debug/engine/lib"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/out/Debug/engine/lib"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/out/Debug/engine/bin"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/out/Release/engine/lib"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/out/Release/engine/lib"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/out/Release/engine/bin"
)
