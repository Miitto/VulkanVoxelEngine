function(find_vulkan)
  find_package (Vulkan REQUIRED)

  # set up Vulkan C++ module
  add_library(VulkanCppModule)
  add_library(Vulkan::cppm ALIAS VulkanCppModule)

  target_compile_definitions(VulkanCppModule PUBLIC
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
        VULKAN_HPP_NO_EXCEPTIONS=1
)
  target_include_directories(VulkanCppModule
        PRIVATE
        "${Vulkan_INCLUDE_DIR}"
)
  target_link_libraries(VulkanCppModule
        PUBLIC
        Vulkan::Vulkan
)

  set_target_properties(VulkanCppModule PROPERTIES CXX_STANDARD 23)

  target_sources(VulkanCppModule
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        BASE_DIRS
        "${Vulkan_INCLUDE_DIR}"
        FILES
        "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm"
)
endfunction()

function(link_vulkan target ACCESS)
  target_link_libraries(${target} ${ACCESS} Vulkan::cppm)
  target_precompile_headers(${target} ${ACCESS}
    <vulkan/vulkan_raii.hpp>
  )
endfunction()
