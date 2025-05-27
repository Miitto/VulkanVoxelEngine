module;

#include <vulkan/vulkan_core.h>

export module vk;

#define EI export import

// Buffers
EI :buffer;
EI :buffers.vertex;
EI :buffers.index;
EI :buffers.uniform;

// Commands
EI :commands.buffer;
EI :commands.pool;

// Device
EI :device;
EI :memory;
EI :device.physical;

// Pipeline
EI :pipeline.graphics;
EI :pipeline.layout;
EI :pipeline;
EI :renderPass;

// Shaders
EI :shader;
EI :shader.mod;

// Structs
EI :attachmentDescription;
EI :bufferCopy;
EI :mappedMemoryRange;
EI :pipelineColorBlendAttachmentState;
EI :subpassDescription;
EI :subpassDependency;
EI :vertexInputAttributsDescription;
EI :vertexInputBindingDescription;
// Info
EI :info.buffers.create;
EI :info.commands.bufferAllocate;
EI :info.commands.bufferBegin;
EI :info.commands.poolCreate;
EI :info.commands.renderPassBegin;
EI :info.pipeline.colorBlendStateCreate;
EI :info.pipeline.dynamicStateCreate;
EI :info.pipeline.graphicsPipelineCreate;
EI :info.pipeline.inputAssemblyStateCreate;
EI :info.pipeline.layoutCreate;
EI :info.pipeline.multisampleStateCreate;
EI :info.pipeline.rasterizationStateCreate;
EI :info.pipeline.shaderStageCreate;
EI :info.pipeline.vertexInputStateCreate;
EI :info.pipeline.viewportStateCreate;
EI :info.sync.fenceCreate;
EI :info.sync.semaphoreCreate;
EI :info.application;
EI :info.deviceCreate;
EI :info.deviceQueueCreate;
EI :info.framebufferCreate;
EI :info.imageViewCreate;
EI :info.instanceCreate;
EI :info.memoryAllocate;
EI :info.present;
EI :info.renderPassCreate;
EI :info.shaderModuleCreate;
EI :info.submit;
EI :info.swapchainCreate;

// Sync
EI :fence;
EI :semaphore;

// Root
EI :descriptors;
EI :framebuffer;
EI :instance;
EI :queue;
EI :surface;
EI :swapchain;
EI :window;

EI :types;
EI :version;
