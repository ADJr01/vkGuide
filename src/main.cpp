#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include "vulkan/RenderV.h"

GLFWwindow* Window;
RenderV renderV;
void initWindow(std::string title="Test Window",int width=800,int height=600) {
    if (!glfwInit()) {
        throw std::runtime_error("GLFW initialization failed");
        return;
    }
    glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API); // we're telling glfw not to work with OpenGL
    glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);
    Window = glfwCreateWindow(width,height,title.c_str(),nullptr,nullptr);

}
int main() {
    try {
        initWindow("Hello Window",800,600);
        if (renderV.init(Window) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }

        while (!glfwWindowShouldClose(Window)) {
            glfwPollEvents();
        }

        glfwDestroyWindow(Window);
        glfwTerminate();
    }catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        glfwDestroyWindow(Window);
        glfwTerminate();
        return -1;
    }

    return 0;
}