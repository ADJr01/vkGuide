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
    // ? Getters
    VkApplicationInfo getAppInfo(std::string appName,std::string engineName);
    void getPhysicalDevice();
    QueueFamilyIndices getQueueFamilies(VkPhysicalDevice& device); // ? for parsing queue families from any physical device
    SwapChainInfo getSwapChainInfo(VkPhysicalDevice device) const;
    // ? setters
    void setupDebugMessenger();
    // ? Check Support Functions
    bool checkInstanceExtensionSupport(const std::vector<const char*>* inputExtensionList);
    bool checkDeviceExtensionSupport(VkPhysicalDevice &device);
    bool checkDeviceSuitability(VkPhysicalDevice physicalDevice);
    void checkPhysicalDeviceInfo(VkPhysicalDevice &device);

    //! Error Callback Function for Vulkan to use
    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                  void* pUserData) {
        const auto debug = ">>vkDebug: ";
        if (messageSeverity && (isSetBit(messageSeverity,1)
            ||
            isSetBit(messageSeverity,2)
            ||
            isSetBit(messageSeverity,3))){
            // For the time being just log all info or error strings
            auto message = std::string(pCallbackData->pMessage);
            std::cerr << debug << message << std::endl;
        }
        return  VK_FALSE;
    }
public:
        RenderV()=default;
        ~RenderV();
        int init(GLFWwindow* window);

};



#endif //RENDERV_H
