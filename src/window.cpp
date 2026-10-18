#include "mge/window.hpp"

#if !defined(NDEBUG)
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif
#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"
#include "utils.h"

#if !defined(NDEBUG)
#include <iostream>
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif
#include <stdexcept>

namespace mge
{
#pragma region VulkanVariables
	vk::Instance instance;
	vk::SurfaceKHR surface;
	vk::Device device;
	vk::CommandPool commandPool;
	vk::CommandBuffer commandBuffer;
#pragma endregion

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
		try
		{
#if !defined(NDEBUG)
			VULKAN_HPP_DEFAULT_DISPATCHER.init(::vkGetInstanceProcAddr);
#endif
			{
				vk::ApplicationInfo appInfo(name.c_str(), VK_MAKE_VERSION(1, 0, 0), "MaNieto's Game Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_3);

				vk::InstanceCreateInfo instanceInfo({}, &appInfo);

				instance = vk::createInstance(instanceInfo);
			}
#if !defined(NDEBUG)
			VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

			std::cout << "Vulkan instance created successfully.\n";
#endif
      if(!glfwInit())
        throw std::runtime_error("Could not initialize GLFW.");

			window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);

      if (window == nullptr)
        throw std::runtime_error("Could not create window.");
#if !defined(NDEBUG)
      std::cout << "Window created successfully.\n";
#endif

			{
				VkSurfaceKHR _surface;
				glfwCreateWindowSurface(instance, window, nullptr, &_surface);
				surface = vk::SurfaceKHR(_surface);
			}

			for (vk::PhysicalDevice physicalDevice : instance.enumeratePhysicalDevices())
			{
				std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

				size_t graphicsQueueFamilyIndex;

				for (graphicsQueueFamilyIndex = 0; graphicsQueueFamilyIndex < queueFamilyProperties.size(); ++graphicsQueueFamilyIndex)
				{
					vk::QueueFamilyProperties qfp = queueFamilyProperties[graphicsQueueFamilyIndex];
					if (qfp.queueFlags & vk::QueueFlagBits::eGraphics)
						break;
				}

				if (graphicsQueueFamilyIndex >= queueFamilyProperties.size())
					continue;

				float queuePriority = 0.0f;
				vk::DeviceQueueCreateInfo deviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), static_cast<uint32_t>(graphicsQueueFamilyIndex), 1, &queuePriority);
				device = physicalDevice.createDevice(vk::DeviceCreateInfo(vk::DeviceCreateFlags(), deviceQueueCreateInfo));
#if !defined(NDEBUG)
				std::cout << "Selected graphics device:\n    " << physicalDevice.getProperties().deviceName << '\n';
#endif

				vk::CommandPoolCreateInfo commandPoolInfo(vk::CommandPoolCreateFlags(), graphicsQueueFamilyIndex);
				commandPool = device.createCommandPool(commandPoolInfo);
#if !defined(NDEBUG)
				std::cout << "Command pool created successfully.\n";
#endif
			}

			if (device == VK_NULL_HANDLE)
				throw std::runtime_error("Not suitable device found.");

			{
				vk::CommandBufferAllocateInfo commandBufferAllocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);

				commandBuffer = device.allocateCommandBuffers(commandBufferAllocateInfo).front();
#if !defined(NDEBUG)
				std::cout << "Command buffer allocated successfully.\n";
#endif
			}
		}
		catch (const vk::SystemError &e)
		{
			throw std::runtime_error(e.what());
		}
	}

	bool Window::ShouldClose()
	{
		return glfwWindowShouldClose(window);
	}

	void Window::Close()
	{
		glfwSetWindowShouldClose(window, true);
	}

	void Window::Destroy()
	{
		if (device != VK_NULL_HANDLE)
		{
			if (commandPool != VK_NULL_HANDLE)
				device.destroyCommandPool(commandPool);

			device.destroy();
		}
		if (instance != VK_NULL_HANDLE)
			instance.destroy();

    glfwTerminate();
	}
}
