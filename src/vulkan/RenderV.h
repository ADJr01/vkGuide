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
    //vulkan functions
    void createVulkanInstance();

public:
        RenderV();
        ~RenderV();
        int init(GLFWwindow* window);

};



#endif //RENDERV_H
