//
// Created by adnan on 6/24/25.
//
#define GLFW_INCLUDE_VULKAN
#include "RenderV.h"

#include <GLFW/glfw3.h>
#include <assert.h>

#include <array>
#include <cstring>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>

VkApplicationInfo RenderV::getAppInfo(std::string appName,
                                      std::string engineName) {
  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = std::move(appName).c_str();
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = std::move(engineName).c_str();
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_2;
  return appInfo;
}

bool RenderV::checkInstanceExtensionSupport(
    const std::vector<const char *> *inputExtensionList) {
  uint32_t extCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
  std::vector<VkExtensionProperties> extensionHolder =
      std::vector<VkExtensionProperties>(extCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extCount,
                                         extensionHolder.data());

  for (const auto &current_extension : *inputExtensionList) {
    auto found = false;
    for (const auto &ext : extensionHolder) {
      if (strcmp(ext.extensionName, current_extension) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      std::cerr << "VkInstanceError: Extension " << current_extension
                << " not found!" << std::endl;
      return found;
    }
  }

  return true;
}

QueueFamilyIndices RenderV::getQueueFamilies(VkPhysicalDevice &device) {
  QueueFamilyIndices Indices;
  uint32_t totalQueueFamilySupport = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(
      device, &totalQueueFamilySupport,
      nullptr);  //! get queue family support in any specefic physical device
  if (totalQueueFamilySupport < 1)
    throw std::runtime_error("Device Don't support any Queue Family");
  auto queueFamilyList =
      std::vector<VkQueueFamilyProperties>(totalQueueFamilySupport);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &totalQueueFamilySupport,
                                           queueFamilyList.data());
  for (uint8_t i = 0; i < queueFamilyList.capacity(); i++) {
    const auto queueFamily = queueFamilyList[i];
    //? checking if queue family has at least one queue then checking if  first
    // byte of queueFlags binary is 1 using bit manipulation
    if (queueFamily.queueCount > 0 &&
        queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      VkBool32 does_support_presentation = VK_FALSE;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->surface,
                                           &does_support_presentation);
      if (does_support_presentation != VK_TRUE) {
        std::cerr << "Queue family does not support presentation!" << std::endl;
        throw std::runtime_error("Queue family does not support presentation!");
      }
      Indices.presentFamily = i;
      Indices.graphicsFamily = i;
      if (Indices.isValidGraphicsFamily() && Indices.isValidPresentFamily())
        break;
    }
  }

  return Indices;
}

//* checking device suitability
bool RenderV::checkDeviceSuitability(VkPhysicalDevice physicalDevice) {
  auto indecies = this->getQueueFamilies(physicalDevice);
  if (!this->checkDeviceExtensionSupport(physicalDevice)) return false;
  const SwapChainInfo swapChainInfo = this->getSwapChainInfo(physicalDevice);
  return indecies.isValidGraphicsFamily() &&
         !swapChainInfo.presentationModes.empty() &&
         !swapChainInfo.surfaceFormats.empty();
}

//* checking device extension support
bool RenderV::checkDeviceExtensionSupport(VkPhysicalDevice &device) {
  uint32_t ext_count = 0;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &ext_count, nullptr);
  if (ext_count < 0) throw std::runtime_error("Device extension not found!");
  std::vector<VkExtensionProperties> extensionHolder =
      std::vector<VkExtensionProperties>(ext_count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &ext_count,
                                       extensionHolder.data());
  for (const auto &ext : this->deviceExtensions) {
    auto found = false;
    for (const auto &available_ext : extensionHolder) {
      if (strcmp(available_ext.extensionName, ext) == 0) {
        found = true;
        break;
      }
    }
    if (!found) return false;
  }
  return true;
}

void RenderV::checkPhysicalDeviceInfo(VkPhysicalDevice &device) {
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(device, &properties);
  std::cout << "Device Name: " << properties.deviceName << std::endl;
  std::cout << "Device Type: " << properties.deviceType << std::endl;
  std::cout << "Driver Version: " << properties.driverVersion << std::endl;
  std::cout << "Vendor ID: " << properties.vendorID << std::endl;
}

void RenderV::createVulkanInstance() {
  // extensions count instance
  uint32_t extensionCount = 0;
  const char **extensions = glfwGetRequiredInstanceExtensions(
      &extensionCount);  // If Vulkan is not available on the machine, this
                         // function returns NULL
  if (extensions == nullptr)
    throw std::runtime_error("GLFW API Unavailable Error");
  // Info about Application
  VkApplicationInfo appInfo = this->getAppInfo("Hello Vulkan", "n/a");
  // Info about Vulkan Instance
  VkInstanceCreateInfo vk_info = {};
  vk_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  vk_info.pApplicationInfo = &appInfo;

  std::vector<const char *> extension_list(extensions,
                                           extensions + extensionCount);
  extension_list.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  // check all extension if supported
  if (!this->checkInstanceExtensionSupport(&extension_list)) {
    throw std::runtime_error("VkInstance doesn't support Required Extensions.");
  }
  // set  extensions
  vk_info.enabledExtensionCount = static_cast<uint32_t>(extension_list.size());
  vk_info.ppEnabledExtensionNames = extension_list.data();
  vk_info.enabledLayerCount = static_cast<uint32_t>(0);
  vk_info.ppEnabledLayerNames = nullptr;

  // create Instance
  if (vkCreateInstance(&vk_info, nullptr, &this->Context.Instance) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create Vulkan instance");
  }
}

void RenderV::createSurface() {
  // glfw handling window creation and initializing surface which will be used
  // by swapchain later
  if (glfwCreateWindowSurface(this->Context.Instance, this->Window, nullptr,
                              &this->surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface");
  }
}

void RenderV::createSwapChain() {
  // getting swapchain info from device
  SwapChainInfo swapChainInfo =
      getSwapChainInfo(this->Context.Device.physicalDevice);
  assert(swapChainInfo.surfaceCapabilities.maxImageCount > 0);
  // * 1. Choose Best Format
  VkSurfaceFormatKHR surfaceFormat =
      this->getBestSurfaceFormat(swapChainInfo.surfaceFormats);
  //* 2. Choose Best Presentation Mode
  const VkPresentModeKHR presentMode =
      this->getBestPresentMode(swapChainInfo.presentationModes);
  //* 3. Choose Best Image Resolution
  VkExtent2D swapChainExtent =
      this->chooseSwapExt(swapChainInfo.surfaceCapabilities);
  auto imageCount = static_cast<uint32_t>(
      swapChainInfo.surfaceCapabilities.minImageCount + 1);
  if (swapChainInfo.surfaceCapabilities.maxImageCount < imageCount)
    imageCount = swapChainInfo.surfaceCapabilities.maxImageCount;
  // let's create swapChain Create info
  VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
  swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapChainCreateInfo.surface = this->surface;
  swapChainCreateInfo.minImageCount =
      imageCount;  // Enabling Triple Buffer. 1 front 2 back
  swapChainCreateInfo.imageFormat = surfaceFormat.format;
  swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
  swapChainCreateInfo.imageExtent = swapChainExtent;
  swapChainCreateInfo.imageArrayLayers =
      1;  //* numbers of layers for each image in chain
  swapChainCreateInfo.imageUsage =
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // what attachment we will be using
  swapChainCreateInfo.preTransform =
      swapChainInfo.surfaceCapabilities
          .currentTransform;  // transform to perform on swap chain
  swapChainCreateInfo.compositeAlpha =
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  // how to blend with external graphics
                                          // , like how to blend with things
                                          // outside of window area
  swapChainCreateInfo.clipped =
      VK_TRUE;  // Clipping part of image which are off-screen
  swapChainCreateInfo.presentMode = presentMode;
  QueueFamilyIndices indices =
      getQueueFamilies(this->Context.Device.physicalDevice);
  if (indices.graphicsFamily == indices.presentFamily) {
    // case: graphics and presentation family are same
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainCreateInfo.queueFamilyIndexCount = 0;
    swapChainCreateInfo.pQueueFamilyIndices = nullptr;
  } else {
    // ? Image will be share between queue families concurrently
    uint32_t queueFamilyIndices[] = {
        static_cast<uint32_t>(indices.graphicsFamily),
        static_cast<uint32_t>(indices.presentFamily),
    };
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapChainCreateInfo.queueFamilyIndexCount = 2;
    swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
  }
  //! if we have old swapChain then we will pass it it to oldSwapChain, which is
  //! mainly used when resizing screen
  swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

  //* create swapchain
  if (vkCreateSwapchainKHR(this->Context.Device.logicalDevice,
                           &swapChainCreateInfo, nullptr,
                           &this->swapChain) != VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain");
  }

  //? storing image format and extent
  this->swapChainImageFormat = surfaceFormat.format;
  this->swapChainExtent = swapChainExtent;

  // get image from swapChain
  uint32_t swapChainImageCount = 0;
  vkGetSwapchainImagesKHR(this->Context.Device.logicalDevice, this->swapChain,
                          &swapChainImageCount, nullptr);
  assert(swapChainImageCount > 0);
  std::vector<VkImage> imageList(swapChainImageCount);
  vkGetSwapchainImagesKHR(this->Context.Device.logicalDevice, this->swapChain,
                          &swapChainImageCount, imageList.data());
  assert(!imageList.empty());
  for (const auto image : imageList) {
    SwapChainImage swapChainImage = {};
    swapChainImage.image = image;
    swapChainImage.imageView = this->createImageViews(
        image, this->swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    this->swapChainImages.push_back(swapChainImage);
  }
}

VkImageView RenderV::createImageViews(VkImage img, VkFormat format,
                                      VkImageAspectFlags aspectFlags) {
  VkImageViewCreateInfo imageViewInfo = {};
  imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewInfo.image = img;
  imageViewInfo.format = format;
  imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  imageViewInfo.subresourceRange.aspectMask = aspectFlags;
  imageViewInfo.subresourceRange.baseMipLevel = 0;
  imageViewInfo.subresourceRange.levelCount = 1;
  imageViewInfo.subresourceRange.baseArrayLayer = 0;
  imageViewInfo.subresourceRange.layerCount = 1;
  VkImageView imageView = VK_NULL_HANDLE;
  if (vkCreateImageView(this->Context.Device.logicalDevice, &imageViewInfo,
                        nullptr, &imageView) != VK_SUCCESS) {
    throw std::range_error("failed to create image view");
  }
  return imageView;
}

VkSurfaceFormatKHR RenderV::getBestSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &formats) {
  // may differ based on different implementation.
  //*in this practice session I will use
  //*VkFormat           format:     VK_FORMAT_R8G8B8A8_UNORM
  // VkColorSpaceKHR    colorSpace: VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
  if (formats.size() < 1)
    throw std::logic_error("Invalid Surface Format Error");
  for (const auto &format : formats) {
    if ((format.format == VK_FORMAT_R8G8B8A8_UNORM ||
         format.format == VK_FORMAT_B8G8R8A8_UNORM) &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return format;
    }
  }
  std::cerr << "FALL BACK Surface Format\n ";
  return formats[0];
}

VkPresentModeKHR RenderV::getBestPresentMode(
    const std::vector<VkPresentModeKHR> &presentationModes) {
  if (presentationModes.size() < 1)
    throw std::logic_error("Invalid Present Mode Error");
  for (const auto &presentationMode : presentationModes) {
    if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return presentationMode;
    }
  }
  std::cerr << "FALL BACK PRESENT MODE VK_PRESENT_MODE_FIFO_KHR\n  ";
  // vulkan guaranteed that `VK_PRESENT_MODE_FIFO_KHR` is present
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D RenderV::chooseSwapExt(
    const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width == 0 ||
      capabilities.currentExtent.width >=
          std::numeric_limits<uint32_t>::max() ||
      capabilities.currentExtent.height == 0 ||
      capabilities.currentExtent.height >=
          std::numeric_limits<uint32_t>::max()) {
    std::cerr << "Invalid Extent Error.Fallback to glfwFrameBuffer Option\n";
    int width, height;
    glfwGetFramebufferSize(this->Window, &width, &height);
    uint32_t min_image_width = std::min(capabilities.maxImageExtent.width,
                                        static_cast<uint32_t>(width));
    uint32_t min_image_height = std::min(capabilities.maxImageExtent.height,
                                         static_cast<uint32_t>(height));
    auto customExtent = VkExtent2D{
        .width =
            std::max(min_image_width,
                     static_cast<uint32_t>(capabilities.minImageExtent.width)),
        .height =
            std::max(min_image_height,
                     static_cast<uint32_t>(capabilities.minImageExtent.height)),
    };

    return customExtent;
  }
  return capabilities.currentExtent;
}

void RenderV::createLogicalDevice() {
  //? Get Queue Families From our chosen physical device
  const float HIGHEST_PRIORITY = 1.0;
  // physical device features for logical device to use
  VkPhysicalDeviceFeatures deviceFeatures = {};
  vkGetPhysicalDeviceFeatures(this->Context.Device.physicalDevice,
                              &deviceFeatures);

  QueueFamilyIndices indices =
      this->getQueueFamilies(this->Context.Device.physicalDevice);
  if (!indices.isValidGraphicsFamily())
    throw std::runtime_error("Device doesn't support Required Queue Family");
  // queues that logical device needs to create.queue create info
  VkDeviceQueueCreateInfo queueCreateInfo = {};
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueFamilyIndex =
      indices.graphicsFamily;  //? index of graphics family to create queue from
  queueCreateInfo.queueCount = 1;
  queueCreateInfo.pQueuePriorities =
      &HIGHEST_PRIORITY;  //! QUEUE priority must be between 0.0 and 1.0

  //?info to create logical device
  VkDeviceCreateInfo logicalDeviceCreateInfo = {};
  logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  logicalDeviceCreateInfo.queueCreateInfoCount =
      1;  // number of queues to create
  logicalDeviceCreateInfo.pQueueCreateInfos =
      &queueCreateInfo;  // queue create infos for logical device to use queues
  logicalDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(
      this->deviceExtensions.size());  // we dont need it for device
  logicalDeviceCreateInfo.ppEnabledExtensionNames =
      this->deviceExtensions
          .data();  // we're not using any extensions for our logical device
  logicalDeviceCreateInfo.pEnabledFeatures = &deviceFeatures;
  // creating logical device
  if (vkCreateDevice(this->Context.Device.physicalDevice,
                     &logicalDeviceCreateInfo, nullptr,
                     &this->Context.Device.logicalDevice) != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device");
  }
  //? if we're here that's mean logical device creation successfully
  // ? now we can get the queue created by logical device
  vkGetDeviceQueue(this->Context.Device.logicalDevice, indices.graphicsFamily,
                   0, &this->graphicsQueue);
  // ? setting up presentation family which will work as interface between
  // display and swapchain
  vkGetDeviceQueue(this->Context.Device.logicalDevice, indices.presentFamily, 0,
                   &this->presentationQueue);
}

void RenderV::getPhysicalDevice() {
  uint32_t physicalDeviceCount = 0;
  vkEnumeratePhysicalDevices(this->Context.Instance, &physicalDeviceCount,
                             nullptr);
  if (physicalDeviceCount < 1)
    throw std::runtime_error(
        "Could not detect any physical device");  // ! if no device found
  auto physicalDevices = std::vector<VkPhysicalDevice>(physicalDeviceCount);
  vkEnumeratePhysicalDevices(this->Context.Instance, &physicalDeviceCount,
                             physicalDevices.data());
  for (auto &physical_device : physicalDevices) {
    if (this->checkDeviceSuitability(physical_device)) {
      this->checkPhysicalDeviceInfo(physical_device);
      this->Context.Device.physicalDevice = physical_device;
      break;
    }
  }
}

SwapChainInfo RenderV::getSwapChainInfo(VkPhysicalDevice device) const {
  SwapChainInfo swapChainInfo = {};
  //? getting surface capabilities from physical device
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->surface,
                                            &swapChainInfo.surfaceCapabilities);

  //? getting formats
  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &formatCount,
                                       nullptr);
  if (formatCount < 1)
    throw std::runtime_error("failed to get required surface formats");
  swapChainInfo.surfaceFormats.resize(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &formatCount,
                                       swapChainInfo.surfaceFormats.data());
  //?Presentation Mode
  uint32_t presentModeCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->surface,
                                            &presentModeCount, nullptr);
  if (presentModeCount < 1)
    throw std::runtime_error("failed to get required presentation modes");
  swapChainInfo.presentationModes.resize(presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, this->surface, &presentModeCount,
      swapChainInfo.presentationModes.data());

  return swapChainInfo;
}

void RenderV::createGraphicsPipeline() {
  //? Create Shader Module
  const auto vertexShaderModule = this->createShaderModule("D:/Projects/Personal/CG/vkGuide/src/shader/vertex.spv");
  const auto fragmentShaderModule = this->createShaderModule("D:/Projects/Personal/CG/vkGuide/src/shader/fragment.spv");

  //# VERTEX SHADER STAGE CREATION INFO
  VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {};
  vertexShaderStageCreateInfo.sType =VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertexShaderStageCreateInfo.module = vertexShaderModule;
  vertexShaderStageCreateInfo.pName ="main";  // target function from where to start

  //# FRAGMENT SHADER STAGE CREATION INFO
  VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {};
  fragmentShaderStageCreateInfo.sType =VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragmentShaderStageCreateInfo.module = fragmentShaderModule;
  fragmentShaderStageCreateInfo.pName ="main";  // target function from where to start

  VkPipelineShaderStageCreateInfo shaderStages[] = {
    vertexShaderStageCreateInfo,
    fragmentShaderStageCreateInfo
};
  //# Create GRAPHICS PIPELINE



  //# Vertex Input (put in vertex description)
  VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
  vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
  vertexInputCreateInfo.pVertexBindingDescriptions = nullptr; // * List of vertex Binding Description (data spacing and stride info)
  vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
  vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;  // * List of vertex Attribiute Description

  //# INPUT ASSEMBLY
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
  inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // * how vertices or point will be assembled
  inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE; //* we're telling vulkan that stop drawing current shape, just start a new one

  //# VIEWPORT & SCISSOR
  VkViewport viewport = {}; // ! require for VkPipelineViewportStateCreateInfo
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(this->swapChainExtent.width);
  viewport.height = static_cast<float>(this->swapChainExtent.height);
  viewport.minDepth = 0.0f; // * the closest point (min depth of frame buffer)
  viewport.maxDepth = 1.0f;//* fartest point (max depth of frame buffer)

  //* creating scissor info
  VkRect2D scissor = {}; // ! require for VkPipelineViewportStateCreateInfo
  scissor.offset = {0,0}; //? offset to use region form
  scissor.extent = this->swapChainExtent;//? available region to use

  VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
  viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportStateCreateInfo.viewportCount = 1;
  viewportStateCreateInfo.pViewports = &viewport;
  viewportStateCreateInfo.scissorCount = 1;
  viewportStateCreateInfo.pScissors = &scissor;

  //# RASTERIZER
  VkPipelineRasterizationStateCreateInfo  rasterizerCreateInfo = {};
  rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizerCreateInfo.depthClampEnable = VK_FALSE; //* controls near and far planes of the viewport, know as depth clipping.
  rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
  rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; //* how to paint the surface of the polygon. we can use VK_POLYGON_MODE_FILL for wireframe effect. But we need GPU feature
  rasterizerCreateInfo.lineWidth = 1.0f; //* how thick the line should be.other than 1.0 we need GPU feature
  rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT; //* which face to cull/skip
  rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; //* Winding to determine which side is front
  rasterizerCreateInfo.depthBiasClamp = VK_FALSE; //* whether to add depth bias to fragment (require for shadow mapping)

  //# MULTISAMPLING
  VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
  multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampleCreateInfo.sampleShadingEnable = VK_FALSE; //!Currently we're disabling multisampling
  multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  //# Blending (how to blend multiple color in fragment)

  VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
  colorBlendAttachmentState.colorWriteMask =  VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |  VK_COLOR_COMPONENT_B_BIT |  VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachmentState.blendEnable = VK_TRUE; // Enable Color Blending
  //* Blend Equation: (VK_BLEND_FACTOR_SRC_ALPHA * newColor) + (VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * oldColor)
  colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {};
  colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlendCreateInfo.logicOpEnable = VK_FALSE; //? AAlternative to calculation is to use logic operation
  colorBlendCreateInfo.attachmentCount = 1;
  colorBlendCreateInfo.pAttachments = &colorBlendAttachmentState;

  //* Pipeline Layout ( TODO: Apply Future Descriptor set layout)
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
  pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutCreateInfo.setLayoutCount = 0;
  pipelineLayoutCreateInfo.pSetLayouts = nullptr;
  pipelineLayoutCreateInfo.pushConstantRangeCount=0;
  pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
  if (vkCreatePipelineLayout(this->Context.Device.logicalDevice,&pipelineLayoutCreateInfo,nullptr,&this->pipelineLayout)!=VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout");
  }

  //TODO: SETUP DEPTH & STENCIL TESTING
  VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
  graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  graphicsPipelineCreateInfo.stageCount = 2; //? number of shader stages
  graphicsPipelineCreateInfo.pStages = shaderStages; //? shaders
  graphicsPipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
  graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
  graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
  graphicsPipelineCreateInfo.pDynamicState = nullptr;
  graphicsPipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
  graphicsPipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
  graphicsPipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
  graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
  graphicsPipelineCreateInfo.layout = pipelineLayout; //?pipeline layout
  graphicsPipelineCreateInfo.renderPass = renderPass; //?render pass description
  graphicsPipelineCreateInfo.subpass = 0;
  //* PIPELINE DERIVATIVES TO CREATE MULTIPLE PIPELINE THAT DERIVE FROM ONE ANOTHER FOR OPTIMIZATION
  graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  graphicsPipelineCreateInfo.basePipelineIndex = -1;

  if (vkCreateGraphicsPipelines(this->Context.Device.logicalDevice,VK_NULL_HANDLE,1,&graphicsPipelineCreateInfo,nullptr,&this->graphicsPipeline)!=VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline");
  }




  //! DESTROY SHADER MODULE AFTER PIPELINE CREATION
  vkDestroyShaderModule(this->Context.Device.logicalDevice,
                        fragmentShaderModule, nullptr);
  vkDestroyShaderModule(this->Context.Device.logicalDevice, vertexShaderModule,
                        nullptr);

}

VkShaderModule RenderV::createShaderModule(std::string shaderPath) {
  const auto shader = RenderV::parseSpirV(shaderPath);
  VkShaderModuleCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = shader.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(shader.data());
  VkShaderModule shaderModule = VK_NULL_HANDLE;
  if (vkCreateShaderModule(this->Context.Device.logicalDevice, &createInfo,
                           nullptr, &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module");
  }
  return shaderModule;
}

void RenderV::createRenderPass() {
  //*create color attachment of render pass
  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = this->swapChainImageFormat;  //?format to use in attachment
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;  //? Count of sample to learn for multisampling
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //? what to do with attachment before rendering
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //? what to do with attachment after rendering
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; //? what to do with stencil before rendering
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;//? what to do with stencil after rendering
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //? image data layout before render pass start
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;//? image data will change to it after render pass

  //* Attachment Reference uses an index that refers to index in attachment list passes into VkRenderPassCreateInfo
  VkAttachmentReference colorAttachmentReference = {};
  colorAttachmentReference.attachment = 0;
  colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  //#attaching subpass: information about particular subpass
  VkSubpassDescription subPassDescription = {};
  subPassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //? binding to Graphics Pipeline
  subPassDescription.colorAttachmentCount = 1;
  subPassDescription.pColorAttachments = &colorAttachmentReference;
  //* need to  handle layout transition using subpass dependencies
  std::array<VkSubpassDependency,2> subpassDependencies{};
  //$ Convertion From VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
  //*transition must happen after
  subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  //*transition must happen before
  subpassDependencies[0].dstSubpass = 0;
  subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpassDependencies[0].dependencyFlags = 0;
  //$ Convertion From VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
  //*transition must happen after
  subpassDependencies[1].srcSubpass = 0;
  subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  //*transition must happen before
  subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  subpassDependencies[1].dstAccessMask =  VK_ACCESS_MEMORY_READ_BIT;
  subpassDependencies[1].dependencyFlags = 0;


  //*Render Pass Create Info
  VkRenderPassCreateInfo renderpassCreateInfo = {};
  renderpassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderpassCreateInfo.attachmentCount = 1;
  renderpassCreateInfo.pAttachments = &colorAttachment;
  renderpassCreateInfo.subpassCount = 1;
  renderpassCreateInfo.pSubpasses = &subPassDescription;
  renderpassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
  renderpassCreateInfo.pDependencies = subpassDependencies.data();
  if (vkCreateRenderPass(this->Context.Device.logicalDevice,&renderpassCreateInfo,nullptr,&this->renderPass)!=VK_SUCCESS) {
      throw std::runtime_error("Failed to create render pass");
  }


}

void RenderV::createFrameBuffers() {
  const auto sizeOfFrameBuffer = this->swapChainImages.size();
  this->swapChainFrameBuffers.resize(sizeOfFrameBuffer);
  int i = 0;
  for (auto& image : this->swapChainImages) {
    std::array<VkImageView,1> attachments = {image.imageView};
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.renderPass = this->renderPass;
    framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferCreateInfo.pAttachments = attachments.data();
    framebufferCreateInfo.width = this->swapChainExtent.width;
    framebufferCreateInfo.height = this->swapChainExtent.height;
    framebufferCreateInfo.layers = 1;
    if (vkCreateFramebuffer(this->Context.Device.logicalDevice,&framebufferCreateInfo,nullptr,&this->swapChainFrameBuffers[i])!=VK_SUCCESS) {
      throw std::runtime_error("Failed to create framebuffer");
    };
    i++;
  }
}


void RenderV::createCMDPool() {
  VkCommandPoolCreateInfo poolCreateInfo;
  poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolCreateInfo.queueFamilyIndex =  getQueueFamilies(this->Context.Device.physicalDevice).graphicsFamily;
  //?Create Graphics Queue Family Command Pool
  if (vkCreateCommandPool(this->Context.Device.logicalDevice,&poolCreateInfo,nullptr,&this->graphicsCMDPool)!=VK_SUCCESS) {
    throw std::runtime_error("Failed to create graphics command pool");
  }
}

void RenderV::createCommandBuffers() {
  const auto size_of_frame_buffer = static_cast<uint32_t>(this->swapChainImages.size());
  this->commandBuffers.resize(size_of_frame_buffer);
  VkCommandBufferAllocateInfo cmdAllocateInfo = {};
  cmdAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmdAllocateInfo.commandPool = this->graphicsCMDPool;
  cmdAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //* Execution order:VK_COMMAND_BUFFER_LEVEL_PRIMARY  signature that it will be executed by queue not other command buffer
  cmdAllocateInfo.commandBufferCount = size_of_frame_buffer;

  if (vkAllocateCommandBuffers(this->Context.Device.logicalDevice,&cmdAllocateInfo,this->commandBuffers.data())!=VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffers");
  }
}

void RenderV::recordCommands() {
  const auto cmdBufferSize = this->swapChainImages.size();
  VkCommandBufferBeginInfo cmdBeginInfo = {};
  cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
  //* info about begin render pass
  for (int i = 0; i < cmdBufferSize; ++i) {
    vkBeginCommandBuffer(this->commandBuffers[i],&cmdBeginInfo)!=VK_SUCCESS?
    throw std::runtime_error("failed to begin recording command buffers"):0;
    //*do tasks

    vkEndCommandBuffer(this->commandBuffers[i])!=VK_SUCCESS?
    throw std::runtime_error("failed to stop recording command buffers"):0;
  }
}





int RenderV::init(GLFWwindow *window) {
  try {
    this->Window = window;
    this->createVulkanInstance();
    this->createSurface();
    this->getPhysicalDevice();
    this->createLogicalDevice();
    this->createSwapChain();
    this->createRenderPass();
    this->createGraphicsPipeline();
  } catch (const std::runtime_error &e) {
    const auto errorMessage = e.what();
    std::cerr << "Runtimer Error: " << errorMessage << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

RenderV::~RenderV() {

  vkDestroyCommandPool(this->Context.Device.logicalDevice,this->graphicsCMDPool,nullptr);
  for (auto framebuffer : this->swapChainFrameBuffers) {
    vkDestroyFramebuffer(this->Context.Device.logicalDevice,framebuffer,nullptr);

  }
  vkDestroyPipeline(this->Context.Device.logicalDevice,this->graphicsPipeline,nullptr);
  vkDestroyRenderPass(this->Context.Device.logicalDevice,this->renderPass,nullptr);
  vkDestroyPipelineLayout(this->Context.Device.logicalDevice,this->pipelineLayout,nullptr);
  for (const auto &img : this->swapChainImages) {
    vkDestroyImageView(this->Context.Device.logicalDevice, img.imageView,
                       nullptr);
  }
  vkDestroySwapchainKHR(this->Context.Device.logicalDevice, this->swapChain,
                        nullptr);
  vkDestroySurfaceKHR(this->Context.Instance, this->surface, nullptr);
  if (this->Context.Device.logicalDevice != VK_NULL_HANDLE)
    vkDestroyDevice(this->Context.Device.logicalDevice, nullptr);
  if (this->Context.Instance != VK_NULL_HANDLE)
    vkDestroyInstance(this->Context.Instance, nullptr);
}

std::vector<char> RenderV::parseSpirV(const std::string &file_path) {
  FILE *file = fopen(file_path.c_str(), "rb");
  if (!file) {
    throw std::runtime_error("Failed to open file");
  }
  if (fseek(file, 0, SEEK_END) != 0) {
    fclose(file);
    throw std::runtime_error("Failed to seek to end of file");
  }
  long size = ftell(file);
  if (size == -1) {
    fclose(file);
    throw std::runtime_error("Failed to get file size");
  }
  if (fseek(file, 0, SEEK_SET) != 0) {
    fclose(file);
    throw std::runtime_error("Failed to seek to beginning of file");
  }
  std::vector<char> buffer(size);
  size_t readSize = fread(buffer.data(), 1, size, file);
  fclose(file);

  if (readSize != static_cast<size_t>(size)) {
    throw std::runtime_error("Failed to read complete file");
  }
  return buffer;
}
