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

#include "../../RenderVUtil.h"

const bool enable_validation_layers = true;

class RenderV {
private:
    GLFWwindow* Window;
    //vulkan Components
    VkInstance Instance;
    struct {
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
    } Device;
    std::vector<const char*> validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };
//! vulkan functions
    // ? Create Functions
    void createVulkanInstance();
    void createLogicalDevice();
    // ? Getters
    VkApplicationInfo getAppInfo(std::string appName,std::string engineName);
    void getPhysicalDevice();
    QueueFamilyIndices getQueueFamilies(VkPhysicalDevice& device); // ? for parsing queue families from any physical device
    // ? setters
    VkQueue  graphicsQueue; //? To store graphics queue created by logical device

    // ? Support Functions
    bool checkInstanceExtensionSupport(const std::vector<const char*>* inputExtensionList);
    bool checkDeviceSuitability(VkPhysicalDevice physicalDevice);
    bool checkValidationLayerSupport() const; // we will check if validation layer is supported
public:
        RenderV()=default;
        ~RenderV();
        int init(GLFWwindow* window);

};



#endif //RENDERV_H
