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


class RenderV {
private:
    GLFWwindow* Window;
    //vulkan Components
    VkInstance Instance;
    struct {
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
    } Device;
//! vulkan functions
    // ? Create Functions
    void createVulkanInstance();

    // ? Getters
    VkApplicationInfo getAppInfo(std::string appName,std::string engineName);
    void getPhysicalDevice();
    QueueFamilyIndices getQueueFamilies(VkPhysicalDevice& device); // ? for parsing queue families from any physical device
    // ? setters


    // ? Support Functions
    bool checkInstanceExtensionSupport(const std::vector<const char*>* inputExtensionList);
    bool checkDeviceSuitability(VkPhysicalDevice physicalDevice);
public:
        RenderV()=default;
        ~RenderV();
        int init(GLFWwindow* window);

};



#endif //RENDERV_H
