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
    bool isValidGraphicsFamily() {
        return graphicsFamily >=0;
    }
};



#endif //RENDERVUTIL_H
