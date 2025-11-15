add_library(logger INTERFACE)
add_library(logger::logger ALIAS logger)

include(spdlog)
link_spdlog(logger INTERFACE)

target_sources(logger INTERFACE FILE_SET HEADERS)
