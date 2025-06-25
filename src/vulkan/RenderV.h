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
    VkApplicationInfo getAppInfo(std::string appName,std::string engineName);
    void getPhysicalDevice();

    // ? Support Functions
    bool checkInstanceExtensionSupport(const std::vector<const char*>* inputExtensionList);
public:
        RenderV()=default;
        ~RenderV();
        int init(GLFWwindow* window);

};



#endif //RENDERV_H
