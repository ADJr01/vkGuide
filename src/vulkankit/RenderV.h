//
// Created by adnan on 6/24/25.
//
#pragma once
#ifndef RENDERV_H
#define RENDERV_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <iostream>
#include "Helper.h"
#include "RenderVUtil.h"

const bool enable_validation_layers = true;

class RenderV {
private:
    GLFWwindow* Window;
    //vulkan Components
    VkContext Context;
    VkQueue  graphicsQueue; //? To store graphics queue created by logical device
    VkQueue presentationQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    const std::vector<const char*> validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };
    // * DEVICE EXTENSIONS
    const std::vector<const char*> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
//! vulkan functions
    // ? Create Functions
    void createVulkanInstance();
    void createLogicalDevice();
    void createSurface();
    void createSwapChain();
    // ? Getters
    VkApplicationInfo getAppInfo(std::string appName,std::string engineName);
    void getPhysicalDevice();
    QueueFamilyIndices getQueueFamilies(VkPhysicalDevice& device); // ? for parsing queue families from any physical device
    SwapChainInfo getSwapChainInfo(VkPhysicalDevice device) const;
    VkSurfaceFormatKHR getBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
    VkPresentModeKHR getBestPresentMode(const std::vector<VkPresentModeKHR>& presentationModes);
    VkExtent2D chooseSwapExt(const VkSurfaceCapabilitiesKHR& capabilities);

    // ? Check Support Functions
    bool checkInstanceExtensionSupport(const std::vector<const char*>* inputExtensionList);
    bool checkDeviceExtensionSupport(VkPhysicalDevice &device);
    bool checkDeviceSuitability(VkPhysicalDevice physicalDevice);
    void checkPhysicalDeviceInfo(VkPhysicalDevice &device);


public:
        RenderV()=default;
        ~RenderV();
        int init(GLFWwindow* window);

};



#endif //RENDERV_H
