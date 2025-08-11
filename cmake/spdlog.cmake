FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.15.3)
FetchContent_MakeAvailable(spdlog)

function(link_spdlog_header_only target)
  target_link_libraries(${target} PRIVATE spdlog::spdlog_header_only)
endfunction()
