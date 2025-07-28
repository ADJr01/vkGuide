//
// Created by adnan on 6/24/25.
//
#pragma once
#ifndef RENDERV_H
#define RENDERV_H
#define GLFW_INCLUDE_VULKAN
#define MAX_FRAMES_IN_FLIGHT 3
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>

#include "Helper.h"
#include "RenderVUtil.h"

const bool enable_validation_layers = true;

class RenderV {
 private:
  int currentFrame = 0;
  GLFWwindow* Window;
  //* vulkan Components
  VkContext Context;
  VkQueue graphicsQueue;  //? To store graphics queue created by logical device
  VkQueue presentationQueue;
  VkSurfaceKHR surface;
  VkSwapchainKHR swapChain;
  VkPipeline graphicsPipeline;
  VkPipelineLayout pipelineLayout;
  VkRenderPass renderPass;
  std::vector<SwapChainImage> swapChainImages;
  std::vector<VkFramebuffer> swapChainFrameBuffers;
  std::vector<VkCommandBuffer> commandBuffers;
   const std::vector<const char*> validation_layers = {
      "VK_LAYER_KHRONOS_validation"};

  //* Pools
  VkCommandPool graphicsCMDPool;
  // * DEVICE EXTENSIONS
  const std::vector<const char*> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  //* Vk Utility
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;

  //* Synchronization
  std::vector<VkSemaphore> imageAvailableSemaphore;
  std::vector<VkSemaphore> renderFinishedSemaphore;

  //? other utility
  static std::vector<char> parseSpirV(const std::string& file_path);

  //! vulkan functions
  // ? Create Functions
  void createVulkanInstance();
  void createLogicalDevice();
  void createSurface();
  void createSwapChain();
  VkShaderModule createShaderModule(std::string shaderPath) const;
  void createGraphicsPipeline();
  void createRenderPass();
  VkImageView createImageViews(VkImage img, VkFormat format,
                               VkImageAspectFlags aspectFlags);
  void createFrameBuffers();
  void createCMDPool();
  void createCommandBuffers();
  void initSemaphores();

  void recordCommands() const;
  // ? Getters
  VkApplicationInfo getAppInfo(std::string appName, std::string engineName);
  void getPhysicalDevice();
  QueueFamilyIndices getQueueFamilies(
      VkPhysicalDevice&
          device);  // ? for parsing queue families from any physical device
  SwapChainInfo getSwapChainInfo(VkPhysicalDevice device) const;
  VkSurfaceFormatKHR getBestSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR>& formats);
  VkPresentModeKHR getBestPresentMode(
      const std::vector<VkPresentModeKHR>& presentationModes);
  VkExtent2D chooseSwapExt(const VkSurfaceCapabilitiesKHR& capabilities);


  // ? Check Support Functions
  bool checkInstanceExtensionSupport(
      const std::vector<const char*>* inputExtensionList);
  bool checkDeviceExtensionSupport(VkPhysicalDevice& device);
  bool checkDeviceSuitability(VkPhysicalDevice physicalDevice);
  void checkPhysicalDeviceInfo(VkPhysicalDevice& device);

 public:
  RenderV() = default;
  ~RenderV();
  int init(GLFWwindow* window);
  void draw();
};

#endif  // RENDERV_H
