//
// Created by adnan on 6/24/25.
//

#include "RenderV.h"

#include <iostream>
RenderV::RenderV() {

}

RenderV::~RenderV() {

}

void RenderV::createVulkanInstance() {
    // Info about Application
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VkPractice";
    appInfo.applicationVersion = VK_MAKE_VERSION(0,1,1);
    appInfo.pEngineName = "Vk" ;
    appInfo.engineVersion = VK_MAKE_VERSION(0,1,1);
    appInfo.apiVersion = VK_API_VERSION_1_2;
    // Info about Vulkan Instance
    VkInstanceCreateInfo vk_info = {};
    vk_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vk_info.pApplicationInfo = &appInfo;

    //create list to hold extension
    auto instanceExtension = std::vector<const char*>();

    // extensions count instance
    uint32_t extensionCount = 0;
    const char** extensions= glfwGetRequiredInstanceExtensions(&extensionCount);
    vk_info.enabledExtensionCount = extensionCount;
    vk_info.ppEnabledExtensionNames = extensions;

    //!Validation Layers
    vk_info.enabledLayerCount = 0;
    vk_info.ppEnabledLayerNames = nullptr;

    // store extension in instanceExtension
    for (int i = 0; i < extensionCount; i++) {
        instanceExtension.push_back(extensions[i]);
    }

}



int RenderV::init(GLFWwindow *window) {
    try {
        this->Window = window;
        this->createVulkanInstance();
    }catch (const std::runtime_error &e) {
        const auto errorMessage = e.what();
        std::cerr<<"Runtimer Error: "<< errorMessage << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

