module;

#include <vulkan/vulkan_core.h>

export module vk;

// Buffers
export import :buffer;
export import :buffers.vertex;
export import :buffers.index;
export import :buffers.uniform;

// Commands
export import :commands.buffer;
export import :commands.pool;

// Device
export import :device;
export import :memory;
export import :device.physical;

// Pipeline
export import :pipeline.graphics;
export import :pipeline.layout;
export import :pipeline;
export import :renderPass;

// Shaders
export import :shader;
export import :shader.mod;

// Structs
export import :attachmentDescription;
export import :bufferCopy;
export import :mappedMemoryRange;
export import :pipelineColorBlendAttachmentState;
export import :subpassDescription;
export import :subpassDependency;
export import :vertexInputAttributeDescription;
export import :vertexInputBindingDescription;
// Info
export import :info.buffers.create;
export import :info.commands.bufferAllocate;
export import :info.commands.bufferBegin;
export import :info.commands.poolCreate;
export import :info.commands.renderPassBegin;
export import :info.pipeline.colorBlendStateCreate;
export import :info.pipeline.dynamicStateCreate;
export import :info.pipeline.graphicsPipelineCreate;
export import :info.pipeline.inputAssemblyStateCreate;
export import :info.pipeline.layoutCreate;
export import :info.pipeline.multisampleStateCreate;
export import :info.pipeline.rasterizationStateCreate;
export import :info.pipeline.shaderStageCreate;
export import :info.pipeline.vertexInputStateCreate;
export import :info.pipeline.viewportStateCreate;
export import :info.sync.fenceCreate;
export import :info.sync.semaphoreCreate;
export import :info.application;
export import :info.deviceCreate;
export import :info.deviceQueueCreate;
export import :info.framebufferCreate;
export import :info.imageViewCreate;
export import :info.instanceCreate;
export import :info.memoryAllocate;
export import :info.present;
export import :info.renderPassCreate;
export import :info.shaderModuleCreate;
export import :info.submit;
export import :info.swapchainCreate;

// Sync
export import :fence;
export import :semaphore;

// Root
export import :descriptors;
export import :framebuffer;
export import :instance;
export import :queue;
export import :surface;
export import :swapchain;
export import :window;

export import :types;
export import :version;
