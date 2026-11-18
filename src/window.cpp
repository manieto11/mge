#include "mge/window.hpp"

#include "utils.h"

#include <format>
#include <iostream>

namespace mge
{
    Window::Window(const std::string name, int width, int height)
    {
        Window::Create(name, width, height);
    }

    Window::~Window()
    {
        Window::Destroy();
    }

    void Window::Create(const std::string name, int width, int height)
    {
        if (window != nullptr)
            throw std::runtime_error("Window already created.");

        if (!glfwInit())
            throw std::runtime_error("GLFW initialization failed.");

        if (!glfwVulkanSupported())
        {
            glfwTerminate();
            throw std::runtime_error("GLFW reports that Vulkan is not available.");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
        application_name = name.c_str();
#ifndef NDEBUG
        std::cout << "Window created: " << width << 'x' << height << ".\n";
#endif

        if (window == nullptr)
        {
            glfwTerminate();
            throw std::runtime_error("GLFW window creation failed.");
        }

        glfwPollEvents();
    }

    bool Window::ShouldClose()
    {
        if (window == nullptr)
            return true;

        glfwPollEvents();
        return glfwWindowShouldClose(window);
    }

    void Window::Close()
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    void Window::Destroy()
    {
        if (window != nullptr)
        {
            glfwDestroyWindow(window);
            window = nullptr;
        }
#ifndef NDEBUG
        std::cout << "Window destroyed.\n";
#endif

        glfwTerminate();
#ifndef NDEBUG
        std::cout << "GLFW terminated.\n";
#endif
    }

    int Window::GetWidth()
    {
        int width;
        glfwGetWindowSize(window, &width, nullptr);
        return width;
    }

    void Window::SetWidth(int width)
    {
        int height;
        glfwGetWindowSize(window, nullptr, &height);
        glfwSetWindowSize(window, width, height);
    }

    int Window::GetHeight()
    {
        int height;
        glfwGetWindowSize(window, nullptr, &height);
        return height;
    }

    void Window::SetHeight(int height)
    {
        int width;
        glfwGetWindowSize(window, &width, nullptr);
        glfwSetWindowSize(window, width, height);
    }
    glm::ivec2 Window::GetSize()
    {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        return glm::ivec2(width, height);
    }

    void Window::SetSize(glm::ivec2 size)
    {
        glfwSetWindowSize(window, size.x, size.y);
    }

    glm::ivec2 Window::GetPosition()
    {
        int x, y;
        glfwGetWindowPos(window, &x, &y);
        return glm::ivec2(x, y);
    }

    void Window::SetPosition(glm::ivec2 position)
    {
        glfwSetWindowPos(window, position.x, position.y);
    }
}