//
// Created by adnan on 6/24/25.
//
#define GLFW_INCLUDE_VULKAN
#include "RenderV.h"

#include <GLFW/glfw3.h>
#include <assert.h>

#include <cstring>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>

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
            VkBool32 does_support_presentation = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device,i,this->surface,&does_support_presentation);
            if (does_support_presentation!=VK_TRUE) {
                std::cerr << "Queue family does not support presentation!" << std::endl;
                throw std::runtime_error("Queue family does not support presentation!");
            }
            Indices.presentFamily = i;
            Indices.graphicsFamily = i;
            if (Indices.isValidGraphicsFamily() && Indices.isValidPresentFamily())break;
        }
    }

    return Indices;
}

//* checking device suitability
bool RenderV::checkDeviceSuitability(VkPhysicalDevice physicalDevice) {
     auto indecies = this->getQueueFamilies(physicalDevice);
    if (!this->checkDeviceExtensionSupport(physicalDevice)) return false;
     const SwapChainInfo swapChainInfo = this->getSwapChainInfo(physicalDevice);
     return indecies.isValidGraphicsFamily() &&  !swapChainInfo.presentationModes.empty() && !swapChainInfo.surfaceFormats.empty();
}

//* checking device extension support
bool RenderV::checkDeviceExtensionSupport(VkPhysicalDevice &device) {
    uint32_t ext_count = 0;
    vkEnumerateDeviceExtensionProperties(device,nullptr,&ext_count,nullptr);
    if (ext_count<0)throw std::runtime_error("Device extension not found!");
    std::vector<VkExtensionProperties> extensionHolder = std::vector<VkExtensionProperties>(ext_count);
    vkEnumerateDeviceExtensionProperties(device,nullptr,&ext_count,extensionHolder.data());
    for (const auto &ext:this->deviceExtensions) {
        auto found = false;
        for (const auto &available_ext:extensionHolder) {
            if (strcmp(available_ext.extensionName,ext)==0) {
                found=true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}


void RenderV::checkPhysicalDeviceInfo(VkPhysicalDevice& device) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device,&properties);
    std::cout<<"Device Name: "<<properties.deviceName<<std::endl;
    std::cout<<"Device Type: "<<properties.deviceType<<std::endl;
    std::cout<<"Driver Version: "<<properties.driverVersion<<std::endl;
    std::cout<<"Vendor ID: "<<properties.vendorID<<std::endl;
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
    extension_list.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    //check all extension if supported
    if (!this->checkInstanceExtensionSupport(&extension_list)) {
        throw std::runtime_error("VkInstance doesn't support Required Extensions.");
    }
    //set  extensions
    vk_info.enabledExtensionCount = static_cast<uint32_t>(extension_list.size());
    vk_info.ppEnabledExtensionNames = extension_list.data();
    vk_info.enabledLayerCount = static_cast<uint32_t>(0);
    vk_info.ppEnabledLayerNames = nullptr;


    //create Instance
    if (vkCreateInstance(&vk_info,nullptr,&this->Context.Instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create Vulkan instance");
    }

}

void RenderV::createSurface() {
    // glfw handling window creation and initializing surface which will be used by swapchain later
    if (glfwCreateWindowSurface(this->Context.Instance,this->Window,nullptr,&this->surface)!=VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }

}

void RenderV::createSwapChain() {
    //getting swapchain info from device
    SwapChainInfo swapChainInfo = getSwapChainInfo(this->Context.Device.physicalDevice);
    assert(swapChainInfo.surfaceCapabilities.maxImageCount>0);
    // * 1. Choose Best Format
    VkSurfaceFormatKHR surfaceFormat = this->getBestSurfaceFormat(swapChainInfo.surfaceFormats);
    //* 2. Choose Best Presentation Mode
    const VkPresentModeKHR presentMode = this->getBestPresentMode(swapChainInfo.presentationModes);
    //* 3. Choose Best Image Resolution
    VkExtent2D swapChainExtent = this->chooseSwapExt(swapChainInfo.surfaceCapabilities);
    auto imageCount =  static_cast<uint32_t>(swapChainInfo.surfaceCapabilities.minImageCount + 1);
    if (swapChainInfo.surfaceCapabilities.maxImageCount<imageCount) imageCount = swapChainInfo.surfaceCapabilities.maxImageCount;
    // let's create swapChain Create info
    VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
      swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = this->surface;
    swapChainCreateInfo.minImageCount = imageCount; // Enabling Triple Buffer. 1 front 2 back
    swapChainCreateInfo.imageFormat = surfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainCreateInfo.imageExtent = swapChainExtent;
    swapChainCreateInfo.imageArrayLayers = 1; //* numbers of layers for each image in chain
    swapChainCreateInfo.imageUsage =  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // what attachment we will be using
    swapChainCreateInfo.preTransform = swapChainInfo.surfaceCapabilities.currentTransform; // transform to perform on swap chain
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // how to blend with external graphics , like how to blend with things outside of window area
    swapChainCreateInfo.clipped = VK_TRUE;     //Clipping part of image which are off screen
    swapChainCreateInfo.presentMode = presentMode;
    QueueFamilyIndices indices = getQueueFamilies(this->Context.Device.physicalDevice);
    if (indices.graphicsFamily==indices.presentFamily) {
      //case: graphics and presentation family are same
      swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      swapChainCreateInfo.queueFamilyIndexCount = 0;
      swapChainCreateInfo.pQueueFamilyIndices = nullptr;
    }else {
      // ? Image will be share between queue families concurrently
      uint32_t queueFamilyIndices[]={
        static_cast<uint32_t>(indices.graphicsFamily),
        static_cast<uint32_t>(indices.presentFamily),
      };
      swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      swapChainCreateInfo.queueFamilyIndexCount = 2;
      swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;

    }
  //! if we have old swapChain then we will pass it it to oldSwapChain, which is mainly used when resizing screen
  swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

  //* create swapchain
  if (vkCreateSwapchainKHR(this->Context.Device.logicalDevice,&swapChainCreateInfo,nullptr,&this->swapChain)!=VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain");

  }
}

VkSurfaceFormatKHR RenderV::getBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats) {
    //may differ based on different implementation.
    //*in this practice session i will use
    //*VkFormat           format:     VK_FORMAT_R8G8B8A8_UNORM
    //VkColorSpaceKHR    colorSpace: VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    if (formats.size()<1) throw std::logic_error("Invalid Surface Format Error");
    for (const auto &format:formats) {
        if ((format.format==VK_FORMAT_R8G8B8A8_UNORM || format.format==VK_FORMAT_B8G8R8A8_UNORM)
            && format.colorSpace==VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    std::cerr<<"FALL BACK Surface Format\n ";
    return formats[0];
}

VkPresentModeKHR RenderV::getBestPresentMode(const std::vector<VkPresentModeKHR> &presentationModes) {
    if (presentationModes.size()<1) throw std::logic_error("Invalid Present Mode Error");
    for (const auto &presentationMode:presentationModes) {
        if (presentationMode==VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentationMode;
        }
    }
    std::cerr<<"FALL BACK PRESENT MODE VK_PRESENT_MODE_FIFO_KHR\n  ";
    // vulkan guaranteed that `VK_PRESENT_MODE_FIFO_KHR` is present
    return  VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D RenderV::chooseSwapExt(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width==0 ||
        capabilities.currentExtent.width>=std::numeric_limits<uint32_t>::max() ||
        capabilities.currentExtent.height==0 || capabilities.currentExtent.height>=std::numeric_limits<uint32_t>::max()
        ) {
        std::cerr<<"Invalid Extent Error.Fallback to glfwFrameBuffer Option\n";
        int width,height;
        glfwGetFramebufferSize(this->Window,&width,&height);
        uint32_t min_image_width = std::min(capabilities.maxImageExtent.width,static_cast<uint32_t>(width));
        uint32_t min_image_height = std::min(capabilities.maxImageExtent.height,static_cast<uint32_t>(height));
        auto customExtent =  VkExtent2D{
            .width = std::max(min_image_width,static_cast<uint32_t>(capabilities.minImageExtent.width)),
            .height =std::max(min_image_height,static_cast<uint32_t>(capabilities.minImageExtent.height)),
        };

        return  customExtent;
    }
    return capabilities.currentExtent;
}





void RenderV::createLogicalDevice() {
    //? Get Queue Families From our chosen physical device
    const float HIGHEST_PRIORITY = 1.0;
    //physical device features for logical device to use
    VkPhysicalDeviceFeatures deviceFeatures = {};
    vkGetPhysicalDeviceFeatures(this->Context.Device.physicalDevice,&deviceFeatures);

    QueueFamilyIndices indices = this->getQueueFamilies(this->Context.Device.physicalDevice);
    if (!indices.isValidGraphicsFamily()) throw std::runtime_error("Device doesn't support Required Queue Family");
    //queues that logical device needs to create.queue create info
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
    logicalDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(this->deviceExtensions.size()); // we dont need it for device
    logicalDeviceCreateInfo.ppEnabledExtensionNames = this->deviceExtensions.data(); // we're not using any extensions for our logical device
    logicalDeviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    //creating logical device
    if (vkCreateDevice(this->Context.Device.physicalDevice,&logicalDeviceCreateInfo,nullptr,&this->Context.Device.logicalDevice)!=VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device");
    }
    //? if we're here that's mean logical device creation successfully
    // ? now we can get the queue created by logical device
    vkGetDeviceQueue(this->Context.Device.logicalDevice,indices.graphicsFamily,0,&this->graphicsQueue);
    // ? setting up presentation family which will work as interface between display and swapchain
    vkGetDeviceQueue(this->Context.Device.logicalDevice,indices.presentFamily,0,&this->presentationQueue);

}



void RenderV::getPhysicalDevice() {
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(this->Context.Instance,&physicalDeviceCount,nullptr);
    if (physicalDeviceCount<1)throw std::runtime_error("Could not detect any physical device");// ! if no device found
    auto physicalDevices = std::vector<VkPhysicalDevice>(physicalDeviceCount);
    vkEnumeratePhysicalDevices(this->Context.Instance,&physicalDeviceCount,physicalDevices.data());
    for (auto &physical_device: physicalDevices) {
        if (this->checkDeviceSuitability(physical_device)) {
            this->checkPhysicalDeviceInfo(physical_device);
            this->Context.Device.physicalDevice = physical_device;
            break;
        }
    }
}

SwapChainInfo RenderV::getSwapChainInfo(VkPhysicalDevice device) const {
    SwapChainInfo swapChainInfo = {};
    //? getting surface capabilities from physical device
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device,this->surface,&swapChainInfo.surfaceCapabilities);

    //? getting formats
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device,this->surface,&formatCount,nullptr);
    if (formatCount<1) throw std::runtime_error("failed to get required surface formats");
    swapChainInfo.surfaceFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device,this->surface,&formatCount,swapChainInfo.surfaceFormats.data());
    //?Presentation Mode
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device,this->surface,&presentModeCount,nullptr);
    if (presentModeCount<1) throw std::runtime_error("failed to get required presentation modes");
    swapChainInfo.presentationModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device,this->surface,&presentModeCount,swapChainInfo.presentationModes.data());


    return swapChainInfo;
}









int RenderV::init(GLFWwindow *window) {
    try {
        this->Window = window;
        this->createVulkanInstance();
        this->createSurface();
        this->getPhysicalDevice();
        this->createLogicalDevice();
        this->createSwapChain();
    }catch (const std::runtime_error &e) {
        const auto errorMessage = e.what();
        std::cerr<<"Runtimer Error: "<< errorMessage << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}




RenderV::~RenderV() {
    vkDestroySwapchainKHR(this->Context.Device.logicalDevice,this->swapChain,nullptr);
    vkDestroySurfaceKHR(this->Context.Instance,this->surface,nullptr);
    if (this->Context.Device.logicalDevice!=VK_NULL_HANDLE) vkDestroyDevice(this->Context.Device.logicalDevice, nullptr);
    if (this->Context.Instance!=VK_NULL_HANDLE) vkDestroyInstance(this->Context.Instance, nullptr);
}

