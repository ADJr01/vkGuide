//
// Created by adnan on 6/26/25.
//

#ifndef RENDERVUTIL_H
#define RENDERVUTIL_H
#include <vulkan/vulkan.h>


typedef  struct {
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
} MainDevice;

typedef struct {
    VkInstance Instance;
    MainDevice Device;
    VkDebugUtilsMessengerEXT debugMessenger;
}VkContext;

struct QueueFamilyIndices {
    int graphicsFamily = -1;
    int presentFamily = -1;
    bool isValidGraphicsFamily() {
        return graphicsFamily >=0;
    }

    bool isValidPresentFamily() {
        return presentFamily >=0;
    }
};

typedef struct {
    VkSurfaceCapabilitiesKHR surfaceCapabilities; //?surface properties, image size, extents etc
    std::vector<VkSurfaceFormatKHR> surfaceFormats; //surface image formats, R8G8B8A8_UNORM
    std::vector<VkPresentModeKHR> presentationModes; // presentation mode...way of Display and SwapChain Synchronization
}SwapChainInfo;



#endif //RENDERVUTIL_H
