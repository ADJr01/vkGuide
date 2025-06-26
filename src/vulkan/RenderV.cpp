//
// Created by adnan on 6/24/25.
//
#define GLFW_INCLUDE_VULKAN
#include "RenderV.h"
#include <cstring>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <iterator>
#include <iostream>




VkApplicationInfo RenderV::getAppInfo(std::string appName, std::string engineName) {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = std::move(appName).c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = std::move(engineName).c_str();
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    return appInfo;
}

bool RenderV::checkInstanceExtensionSupport(const std::vector<const char*>* inputExtensionList) {
    uint32_t extCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
    std::vector<VkExtensionProperties> extensionHolder =  std::vector<VkExtensionProperties>(extCount);
    vkEnumerateInstanceExtensionProperties(nullptr,&extCount,extensionHolder.data());

    for (const auto &current_extension : *inputExtensionList) {
        auto found = false;
        for (const auto &ext:extensionHolder ) {
            if (strcmp(ext.extensionName, current_extension) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            std::cerr << "VkInstanceError: Extension " << current_extension << " not found!" << std::endl;
            return found;
        }
    }

    return true;
}

QueueFamilyIndices RenderV::getQueueFamilies(VkPhysicalDevice& device) {
    QueueFamilyIndices Indices;
    uint32_t totalQueueFamilySupport = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device,&totalQueueFamilySupport,nullptr);//! get queue family support in any specefic physical device
    if (totalQueueFamilySupport<1) throw std::runtime_error("Device Don't support any Queue Family");
    auto queueFamilyList = std::vector<VkQueueFamilyProperties>(totalQueueFamilySupport);
    vkGetPhysicalDeviceQueueFamilyProperties(device,&totalQueueFamilySupport,queueFamilyList.data());
    for (uint8_t i=0;i<queueFamilyList.capacity();i++) {
        const auto queueFamily = queueFamilyList[i];
        //? checking if queue family has at least one queue then checking if  first byte of queueFlags binary is 1 using bit manipulation
        if (queueFamily.queueCount> 0  && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            Indices.graphicsFamily = i;
            if (Indices.isValidGraphicsFamily())break;
        }
    }

    return Indices;
}

bool RenderV::checkDeviceSuitability(VkPhysicalDevice physicalDevice) {
     auto indecies = this->getQueueFamilies(physicalDevice);
    if (!indecies.isValidGraphicsFamily())return false;
    return true;
}



void RenderV::createVulkanInstance() {
    // extensions count instance
    uint32_t extensionCount = 0;
    const char** extensions= glfwGetRequiredInstanceExtensions(&extensionCount);//If Vulkan is not available on the machine, this function returns NULL
    if (extensions==nullptr) throw std::runtime_error("GLFW API Unavailable Error");
    // Info about Application
    VkApplicationInfo appInfo = this->getAppInfo("Hello Vulkan","n/a");
    // Info about Vulkan Instance
    VkInstanceCreateInfo vk_info = {};
    vk_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vk_info.pApplicationInfo = &appInfo;

    std::vector<const char*> extension_list(extensions, extensions + extensionCount);
    //set  extensions
    vk_info.enabledExtensionCount = static_cast<uint32_t>(extension_list.size());
    vk_info.ppEnabledExtensionNames = extension_list.data();
    //check all extension if supported
    if (!this->checkInstanceExtensionSupport(&extension_list)) {
        throw std::runtime_error("VkInstance doesn't support Required Extensions.");

    }
    //!Validation Layers (for the time being we're skipping it)
    vk_info.enabledLayerCount = 0;
    vk_info.ppEnabledLayerNames = nullptr;

    //create Instance
    if (vkCreateInstance(&vk_info,nullptr,&this->Instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create Vulkan instance");
    }

    this->getPhysicalDevice();


}

void RenderV::getPhysicalDevice() {
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(this->Instance,&physicalDeviceCount,nullptr);
    if (physicalDeviceCount<1)throw std::runtime_error("Could not detect any physical device");// ! if no device found
    auto physicalDevices = std::vector<VkPhysicalDevice>(physicalDeviceCount);
    vkEnumeratePhysicalDevices(this->Instance,&physicalDeviceCount,physicalDevices.data());
    for (auto &physical_device: physicalDevices) {
        if (this->checkDeviceSuitability(physical_device)) {
            this->Device.physicalDevice = physical_device;
            std::cout<<"Compatible Physical Device Found\n";
            break;
        }
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


RenderV::~RenderV() {
    vkDestroyInstance(this->Instance, nullptr);
}

