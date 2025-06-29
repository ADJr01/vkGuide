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
    appInfo.apiVersion = VK_API_VERSION_1_2;
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
bool RenderV::checkValidationLayerSupport() const {
    uint32_t validation_layer_count = 0;
    vkEnumerateInstanceLayerProperties(&validation_layer_count, nullptr);
    if (validation_layer_count<1)return false;
    std:: vector<VkLayerProperties> available_layers(validation_layer_count);
    vkEnumerateInstanceLayerProperties(&validation_layer_count, available_layers.data());
    for (const auto &current_layer: this->validation_layers) {
        for (const auto &layer:available_layers) {
            if (strcmp(current_layer,layer.layerName)==0)return true;
        }
    }
    return false;

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
    if (this->checkValidationLayerSupport()) {
        vk_info.enabledLayerCount = static_cast<uint32_t>(this->validation_layers.size());
        vk_info.ppEnabledLayerNames = this->validation_layers.data();
        std::cout<<"Validation Layer Attachet"<<std::endl;
    }else {
        std::cerr<<"Validation Layer Not Supported"<<std::endl;
    }


    //create Instance
    if (vkCreateInstance(&vk_info,nullptr,&this->Instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create Vulkan instance");
    }




}

void RenderV::createLogicalDevice() {
    //? Get Queue Families From our chosen physical device
    const float HIGHEST_PRIORITY = 1.0;
    //physical device features for logical device to use
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.shaderFloat64 = true;

    QueueFamilyIndices indices = this->getQueueFamilies(this->Device.physicalDevice);
    if (!indices.isValidGraphicsFamily()) throw std::runtime_error("Device doesn't support Required Queue Family");
    //queues that logical device need to create.queue create info
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily; //? index of graphics family to create queue from
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities=&HIGHEST_PRIORITY; //! QUEUE priority must be between 0.0 and 1.0

    //?info to create logical device
    VkDeviceCreateInfo logicalDeviceCreateInfo = {};
    logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceCreateInfo.queueCreateInfoCount = 1; // number of queues to create
    logicalDeviceCreateInfo.pQueueCreateInfos = &queueCreateInfo; //queue create infos for logical device to use queues
    logicalDeviceCreateInfo.enabledExtensionCount = 0; // we dont need it for device
    logicalDeviceCreateInfo.ppEnabledExtensionNames = nullptr; // we're not using any extensions for our logical device
    logicalDeviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    //creating logical device
    if (vkCreateDevice(this->Device.physicalDevice,&logicalDeviceCreateInfo,nullptr,&this->Device.logicalDevice)!=VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device");
    }
    //? if we're here that's mean logical device creation successfully
    // ? now we can get the queue created by logical device
    vkGetDeviceQueue(this->Device.logicalDevice,indices.graphicsFamily/*! as we're using graphics family*/,0/*?very first queue*/,&this->graphicsQueue);

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
        this->getPhysicalDevice();
        this->createLogicalDevice();
    }catch (const std::runtime_error &e) {
        const auto errorMessage = e.what();
        std::cerr<<"Runtimer Error: "<< errorMessage << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


RenderV::~RenderV() {
    //vkDestroyDevice(this->Device.logicalDevice,nullptr);
    //vkDestroyInstance(this->Instance, nullptr);
    if (this->Device.logicalDevice!=VK_NULL_HANDLE) {
        vkDestroyDevice(this->Device.logicalDevice, nullptr);
    }
    if (this->Instance!=VK_NULL_HANDLE) {
        vkDestroyInstance(this->Instance, nullptr);
    }
}

