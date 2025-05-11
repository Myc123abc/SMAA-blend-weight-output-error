#include "vulkan/vulkan_core.h"
#include <cstdint>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_init.h>

#include <vulkan/vulkan.h>

#include <vector>
#include <print>
#include <fstream>
#include <string_view>
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
//                              global vars
////////////////////////////////////////////////////////////////////////////////

constexpr uint32_t VK_API_VERTSION = VK_MAKE_VERSION(1, 4, 0);

SDL_Window*              g_window;
VkInstance               g_instance;
VkDebugUtilsMessengerEXT g_debug_messenger;
VkSurfaceKHR             g_surface;
VkPhysicalDevice         g_physical_device;
VkQueue                  g_queue;
uint32_t                 g_queue_family_index;
VkDevice                 g_device;
VmaAllocator             g_allocator;
VkSwapchainKHR           g_swapchain;
VkFormat                 g_swapchain_image_format;
uint32_t                 g_swapchain_image_count;
VkExtent2D               g_swapchain_extent;
std::vector<VkImage>     g_swapchain_images;
std::vector<VkImageView> g_swapchain_image_views;
std::vector<VkPipeline>  g_pipelines(4);  // 0 : triangle, 1 : edge detection, 2 : blend weight, 3 : neighbor
VkPipelineLayout         g_pipeline_layout_triangle;
VkPipelineLayout         g_pipeline_layout_SMAA;  // all SMAA pass use common pipeline layout
VkCommandPool            g_command_pool;

struct Frame
{
  VkCommandBuffer cmd;
  VkFence         fence;
  VkSemaphore     image_available; 
  VkSemaphore     render_finished;
};

std::vector<Frame> g_frames;

////////////////////////////////////////////////////////////////////////////////
//                              misc funcs
////////////////////////////////////////////////////////////////////////////////

inline void exit_if(bool b)
{
  if (b) exit(1);
}

inline void check_vk(VkResult result)
{
  exit_if(result != VK_SUCCESS);
}

void release_resources()
{
  vkDeviceWaitIdle(g_device);
  for (auto& frame : g_frames)
  {
    vkDestroySemaphore(g_device, frame.image_available, nullptr);
    vkDestroySemaphore(g_device, frame.render_finished, nullptr);
    vkDestroyFence(g_device, frame.fence, nullptr);
    vkFreeCommandBuffers(g_device, g_command_pool, 1, &frame.cmd);
  }
  vkDestroyCommandPool(g_device, g_command_pool, nullptr);
  for (auto pipeline : g_pipelines)
    vkDestroyPipeline(g_device, pipeline, nullptr);
  vkDestroyPipelineLayout(g_device, g_pipeline_layout_triangle, nullptr);
  vkDestroyPipelineLayout(g_device, g_pipeline_layout_SMAA, nullptr);
  for (auto image_view : g_swapchain_image_views)
    vkDestroyImageView(g_device, image_view, nullptr);
  vkDestroySwapchainKHR(g_device, g_swapchain, nullptr);
  vmaDestroyAllocator(g_allocator);
  vkDestroyDevice(g_device, nullptr);
  vkDestroySurfaceKHR(g_instance, g_surface, nullptr);
  vkDestroyDebugUtilsMessengerEXT(g_instance, g_debug_messenger, nullptr);
  vkDestroyInstance(g_instance, nullptr);
  SDL_DestroyWindow(g_window);
  SDL_Quit();
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback(
  VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
  VkDebugUtilsMessageTypeFlagsEXT             message_type,
  VkDebugUtilsMessengerCallbackDataEXT const* callback_data,
  void*                                       user_data)
{
  std::println("{}", callback_data->pMessage);
  return VK_FALSE;
}

auto get_debug_info()
{
  return VkDebugUtilsMessengerCreateInfoEXT
  {
    .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    .pfnUserCallback = debug_messenger_callback,
  };
  
}

VkResult vkCreateDebugUtilsMessengerEXT(
  VkInstance                                  instance,
  VkDebugUtilsMessengerCreateInfoEXT const*   pCreateInfo,
  VkAllocationCallbacks const*                pAllocator,
  VkDebugUtilsMessengerEXT*                   pMessenger)
{
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,"vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) 
    return func(instance, pCreateInfo, pAllocator, pMessenger);
  return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void vkDestroyDebugUtilsMessengerEXT(
  VkInstance                                  instance,
  VkDebugUtilsMessengerEXT                    messenger,
  VkAllocationCallbacks const*                pAllocator)
{
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr)
    func(instance, messenger, pAllocator);
}

auto get_file_data(std::string_view filename)
{
  std::ifstream file(filename.data(), std::ios::ate | std::ios::binary);
  exit_if(!file.is_open());

  auto file_size = (size_t)file.tellg();
  // A SPIR-V module is defined a stream of 32bit words
  auto buffer    = std::vector<uint32_t>(file_size / sizeof(uint32_t));
  
  file.seekg(0);
  file.read((char*)buffer.data(), file_size);

  file.close();
  return buffer;
}

auto create_shader_module(std::string_view filename)
{
  auto data = get_file_data(filename);
  VkShaderModuleCreateInfo shader_info
  {
    .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = data.size() * sizeof(uint32_t),
    .pCode    = reinterpret_cast<uint32_t*>(data.data()),
  };
  VkShaderModule shader_module;
  check_vk(vkCreateShaderModule(g_device, &shader_info, nullptr, &shader_module));
  return shader_module;
}

void transform_image_layout(VkCommandBuffer cmd, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout)
{
  VkImageMemoryBarrier barrier
  {
    .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .oldLayout           = old_layout,
    .newLayout           = new_layout,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image               = image,
    .subresourceRange     = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
  };
  vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

////////////////////////////////////////////////////////////////////////////////
//                              init funcs
////////////////////////////////////////////////////////////////////////////////

void init_SDL()
{
  // SDL init
  SDL_Init(SDL_INIT_VIDEO);

  // create SDL window
  exit_if(!(g_window = SDL_CreateWindow("SMAA Test", 500, 500, SDL_WINDOW_VULKAN)));
}

void create_instance()
{
  // app info
  VkApplicationInfo app_info
  {
    .sType      = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .apiVersion = VK_API_VERTSION,
  };

  // enable validation layer
  const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
  auto debug_info = get_debug_info();
  
  // get extensions
  uint32_t count;
  auto ret = SDL_Vulkan_GetInstanceExtensions(&count);
  auto extensions = std::vector(ret, ret + count);
  extensions.emplace_back("VK_EXT_debug_utils");

  // create instance
  VkInstanceCreateInfo instance_info
  { 
    .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext                   = &debug_info,
    .pApplicationInfo        = &app_info,
    .enabledLayerCount       = 1,
    .ppEnabledLayerNames     = layers,
    .enabledExtensionCount   = static_cast<uint32_t>(extensions.size()),
    .ppEnabledExtensionNames = extensions.data(),
  };
  check_vk(vkCreateInstance(&instance_info, nullptr, &g_instance));
}

void create_debug_messenger()
{
  auto debug_info = get_debug_info();
  check_vk(vkCreateDebugUtilsMessengerEXT(g_instance, &debug_info, nullptr, &g_debug_messenger));
}

void create_surface()
{
  exit_if(!SDL_Vulkan_CreateSurface(g_window, g_instance, nullptr, &g_surface));
}

void select_physical_device()
{
  uint32_t count;
  vkEnumeratePhysicalDevices(g_instance, &count, nullptr);
  std::vector<VkPhysicalDevice> devices(count);
  vkEnumeratePhysicalDevices(g_instance, &count, devices.data());
  g_physical_device = devices[0];
  exit_if(!g_physical_device);

}

void create_device_and_get_graphics_queue()
{
  // get queue family properties
  uint32_t count;
  vkGetPhysicalDeviceQueueFamilyProperties(g_physical_device, &count, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(count);
  vkGetPhysicalDeviceQueueFamilyProperties(g_physical_device, &count, queue_families.data());

  // get graphics queue properties
  auto it = std::find_if(queue_families.begin(), queue_families.end(), [](VkQueueFamilyProperties const& queue_family) 
  {
    return queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT;
  });
  exit_if(it == queue_families.end());

  // set graphics queue info
  auto priority = 1.f;
  g_queue_family_index = static_cast<uint32_t>(std::distance(it, queue_families.begin()));
  VkDeviceQueueCreateInfo queue_info
  {
    .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = g_queue_family_index,
    .queueCount       = 1,
    .pQueuePriorities = &priority,
  };

  // features
  VkPhysicalDeviceVulkan13Features features13
  { 
    .sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
    .pNext               = nullptr, 
    .synchronization2    = true,
    .dynamicRendering    = true,
  };
  VkPhysicalDeviceFeatures2 features2
  {
    .sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
    .pNext    = &features13,
  };

  // create device
  const char* extensions[] = { "VK_KHR_swapchain" };
  VkDeviceCreateInfo device_info
  {
    .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext                   = &features2,
    .queueCreateInfoCount    = 1,
    .pQueueCreateInfos       = &queue_info,
    .enabledExtensionCount   = 1,
    .ppEnabledExtensionNames = extensions,
  };
  check_vk(vkCreateDevice(g_physical_device, &device_info, nullptr, &g_device));

  // get graphics queue
  vkGetDeviceQueue(g_device, g_queue_family_index, 0, &g_queue);
}

void init_vma()
{
  VmaAllocatorCreateInfo allocator_info
  {
    .flags            = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT |
                        VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
    .physicalDevice   = g_physical_device,
    .device           = g_device,
    .instance         = g_instance,
    .vulkanApiVersion = VK_API_VERTSION,
  };
  check_vk(vmaCreateAllocator(&allocator_info, &g_allocator));
}

void create_swapchain()
{
  // get surface capabilities
  VkSurfaceCapabilitiesKHR  surface_capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_physical_device, g_surface, &surface_capabilities);

  // get surface formats
  std::vector<VkSurfaceFormatKHR> surface_formats;
  uint32_t count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(g_physical_device, g_surface, &count, nullptr);
  surface_formats.resize(count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(g_physical_device, g_surface, &count, surface_formats.data());

  // create swapchain
  VkSwapchainCreateInfoKHR info
  {
    .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface          = g_surface,
    .minImageCount    = surface_capabilities.minImageCount + 1,
    .imageFormat      = surface_formats[0].format,
    .imageColorSpace  = surface_formats[0].colorSpace,
    .imageExtent      = surface_capabilities.currentExtent,
    .imageArrayLayers = 1,
    .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    .preTransform     = surface_capabilities.currentTransform,
    .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode      = VK_PRESENT_MODE_FIFO_KHR,
    .clipped          = VK_TRUE,
  };
  check_vk(vkCreateSwapchainKHR(g_device, &info, nullptr, &g_swapchain));

  // get swapchain images
  vkGetSwapchainImagesKHR(g_device, g_swapchain, &g_swapchain_image_count, nullptr);
  g_swapchain_images.resize(g_swapchain_image_count);
  vkGetSwapchainImagesKHR(g_device, g_swapchain, &g_swapchain_image_count, g_swapchain_images.data());

  // create image views
  g_swapchain_image_views.resize(g_swapchain_image_count);
  for (size_t i = 0; i < g_swapchain_image_count; ++i)
  {
    VkImageViewCreateInfo info
    {
      .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image            = g_swapchain_images[i],
      .viewType         = VK_IMAGE_VIEW_TYPE_2D,
      .format           = surface_formats[0].format,
      .components       = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
      .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
    };
    check_vk(vkCreateImageView(g_device, &info, nullptr, &g_swapchain_image_views[i]));
  }

  // set swapchain image format
  g_swapchain_image_format = surface_formats[0].format;
  // get swapchain extent
  g_swapchain_extent = surface_capabilities.currentExtent;
}

void create_pipeline_layout()
{
  // create triangle pipeline layout
  VkPipelineLayoutCreateInfo layout_info
  {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
  };
  check_vk(vkCreatePipelineLayout(g_device, &layout_info, nullptr, &g_pipeline_layout_triangle));

  // create SMAA pipeline layout
  check_vk(vkCreatePipelineLayout(g_device, &layout_info, nullptr, &g_pipeline_layout_SMAA));
}

auto create_pipeline(VkPipelineLayout pipeline_layout, std::string_view vertex_shader, std::string_view fragment_shader) -> VkPipeline
{
  // dynamic rendering
  VkPipelineRenderingCreateInfo dynamic_rendering_info
  {
    .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .colorAttachmentCount    = 1,
    .pColorAttachmentFormats = &g_swapchain_image_format,
  };

  // vertex and fragment shaders
  std::vector<VkPipelineShaderStageCreateInfo> shader_stages(2);
  shader_stages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
  shader_stages[0].module = create_shader_module(vertex_shader);
  shader_stages[0].pName  = "main";
  shader_stages[1] = shader_stages[0];
  shader_stages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
  shader_stages[1].module = create_shader_module(fragment_shader);

  // rasterization state
  VkPipelineRasterizationStateCreateInfo rasterization_state
  {
    .sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode    = VK_CULL_MODE_BACK_BIT,
    .frontFace   = VK_FRONT_FACE_CLOCKWISE,
    .lineWidth   = 1.f,
  };

  // dynamic states
  auto dynamic_states = std::vector<VkDynamicState>
  {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
  };
  VkPipelineDynamicStateCreateInfo dynamic_state
  {
    .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
    .pDynamicStates    = dynamic_states.data(),
  };

  // misc states
  VkPipelineVertexInputStateCreateInfo vertex_input_state
  {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
  };
  VkPipelineInputAssemblyStateCreateInfo input_assembly_state
  { 
    .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
  };
  VkPipelineViewportStateCreateInfo viewport_state
  { 
    .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .scissorCount  = 1,
  };
  VkPipelineMultisampleStateCreateInfo multisample_state
  {
    .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
    .minSampleShading      = 1.f,
  };
  VkPipelineDepthStencilStateCreateInfo depth_stencil_state
  {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
  };
  VkPipelineColorBlendAttachmentState color_blend_attachment
  {
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                      VK_COLOR_COMPONENT_G_BIT |
                      VK_COLOR_COMPONENT_B_BIT |
                      VK_COLOR_COMPONENT_A_BIT,
  };
  VkPipelineColorBlendStateCreateInfo color_blend_state
  { 
    .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments    = &color_blend_attachment,
  };

  // create pipeline
  VkPipeline pipeline;
  VkGraphicsPipelineCreateInfo info
  {
    .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext               = &dynamic_rendering_info,
    .stageCount          = static_cast<uint32_t>(shader_stages.size()),
    .pStages             = shader_stages.data(),
    .pVertexInputState   = &vertex_input_state,
    .pInputAssemblyState = &input_assembly_state,
    .pViewportState      = &viewport_state,
    .pRasterizationState = &rasterization_state,
    .pMultisampleState   = &multisample_state,
    .pDepthStencilState  = &depth_stencil_state,
    .pColorBlendState    = &color_blend_state,
    .pDynamicState       = &dynamic_state,
    .layout              = pipeline_layout,
  };
  check_vk(vkCreateGraphicsPipelines(g_device, nullptr, 1, &info, nullptr, &pipeline));

  // destroy shader modules
  vkDestroyShaderModule(g_device, shader_stages[0].module, nullptr);
  vkDestroyShaderModule(g_device, shader_stages[1].module, nullptr);

  return pipeline;
}

void create_command_pool()
{
  VkCommandPoolCreateInfo info
  {
    .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = g_queue_family_index,
  };
  check_vk(vkCreateCommandPool(g_device, &info, nullptr, &g_command_pool));
}

void init_frames()
{
  g_frames.resize(g_swapchain_image_count);
  for (auto& frame : g_frames)
  {
    VkCommandBufferAllocateInfo cmd_info
    {
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool        = g_command_pool,
      .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount  = 1,
    };
    check_vk(vkAllocateCommandBuffers(g_device, &cmd_info, &frame.cmd));

    VkFenceCreateInfo fence_info
    {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    check_vk(vkCreateFence(g_device, &fence_info, nullptr, &frame.fence));

    VkSemaphoreCreateInfo semaphore_info
    {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    check_vk(vkCreateSemaphore(g_device, &semaphore_info, nullptr, &frame.image_available));
    check_vk(vkCreateSemaphore(g_device, &semaphore_info, nullptr, &frame.render_finished));
  }
}

void init_vk()
{
  create_instance();
  create_debug_messenger();
  create_surface();
  select_physical_device();
  create_device_and_get_graphics_queue();
  init_vma();
  create_swapchain();
  create_pipeline_layout();
  g_pipelines[0] = create_pipeline(g_pipeline_layout_triangle, "triangle_vert.spv",            "triangle_frag.spv");
  //g_pipelines[1] = create_pipeline(g_pipeline_layout_SMAA,     "SMAA_edge_detection_vert.spv", "SMAA_edge_detection_frag.spv");
  //g_pipelines[2] = create_pipeline(g_pipeline_layout_SMAA,     "SMAA_blend_weight_vert.spv",   "SMAA_blend_weight_frag.spv");
  //g_pipelines[3] = create_pipeline(g_pipeline_layout_SMAA,     "SMAA_neighbor_vert.spv",       "SMAA_neighbor_frag.spv");
  create_command_pool();
  init_frames();
}

////////////////////////////////////////////////////////////////////////////////
//                              render funcs
////////////////////////////////////////////////////////////////////////////////

void render()
{
  // get current frame
  static int frame_index = 0;
  auto frame = g_frames[frame_index];

  // wait for previous frame
  check_vk(vkWaitForFences(g_device, 1, &frame.fence, VK_TRUE, UINT64_MAX));
  check_vk(vkResetFences(g_device, 1, &frame.fence));

  // acquire next image
  uint32_t image_index;
  check_vk(vkAcquireNextImageKHR(g_device, g_swapchain, UINT64_MAX, frame.image_available, VK_NULL_HANDLE, &image_index));

  // begin command buffer
  check_vk(vkResetCommandBuffer(frame.cmd, 0));
  VkCommandBufferBeginInfo beg_info
  {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  vkBeginCommandBuffer(frame.cmd, &beg_info);

  // render color attachment
  transform_image_layout(frame.cmd, g_swapchain_images[image_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  VkRenderingAttachmentInfo color_attachment
  {
    .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    .imageView   = g_swapchain_image_views[image_index],
    .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    .loadOp      = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
  };

  // begin rendering
  VkRenderingInfo rendering
  {
    .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
    .renderArea           = { {}, g_swapchain_extent },
    .layerCount           = 1,
    .colorAttachmentCount = 1,
    .pColorAttachments    = &color_attachment,
  };
  vkCmdBeginRendering(frame.cmd, &rendering);

  // viewport and scissor
  VkViewport viewport
  {
    .width    = static_cast<float>(g_swapchain_extent.width),
    .height   = static_cast<float>(g_swapchain_extent.height),
    .maxDepth = 1.f
  };
  vkCmdSetViewport(frame.cmd, 0, 1, &viewport);
  VkRect2D scissor{ {}, g_swapchain_extent };
  vkCmdSetScissor(frame.cmd, 0, 1, &scissor);

  // bind pipeline
  vkCmdBindPipeline(frame.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipelines[0]);

  // draw triangle
  vkCmdDraw(frame.cmd, 3, 1, 0, 0);

  // end rendering
  vkCmdEndRendering(frame.cmd);

  // transform sawpchain image to present layout
  transform_image_layout(frame.cmd, g_swapchain_images[image_index], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  // end command buffer
  vkEndCommandBuffer(frame.cmd);

  // submit command
  VkCommandBufferSubmitInfo cmd_submit_info
  {
    .sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
    .commandBuffer = frame.cmd,
  };
  VkSemaphoreSubmitInfo wait_sem_submit_info
  {
    .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
    .semaphore = frame.image_available,
    .value     = 1,
    .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
  };
  auto signal_sem_submit_info      = wait_sem_submit_info;
  signal_sem_submit_info.semaphore = frame.render_finished;
  signal_sem_submit_info.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

  VkSubmitInfo2 submit_info
  {
    .sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
    .waitSemaphoreInfoCount   = 1,
    .pWaitSemaphoreInfos      = &wait_sem_submit_info,
    .commandBufferInfoCount   = 1,
    .pCommandBufferInfos      = &cmd_submit_info,
    .signalSemaphoreInfoCount = 1,
    .pSignalSemaphoreInfos    = &signal_sem_submit_info,
  };
  check_vk(vkQueueSubmit2(g_queue, 1, &submit_info, frame.fence));

  // present
  VkPresentInfoKHR present_info
  {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores    = &frame.render_finished,
    .swapchainCount     = 1,
    .pSwapchains        = &g_swapchain,
    .pImageIndices      = &image_index,
  };
  check_vk(vkQueuePresentKHR(g_queue, &present_info)); 

  // next frame
  frame_index = (frame_index + 1) % g_swapchain_image_count;
}

////////////////////////////////////////////////////////////////////////////////
//                              main func
////////////////////////////////////////////////////////////////////////////////

int main()
{
  init_SDL();
  init_vk();

  bool quit = false;
  while (!quit)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_EVENT_QUIT)
        quit = true;
    }

    render();
  }

  release_resources();
  
  return 0;
}