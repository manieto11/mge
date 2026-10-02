#include "mge/window.h"

#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>

#include <format>
#include <iostream>
#include <stdexcept>
#include <vector>

// #define VK_ENABLE_VALIDATION // Destroy once all the code is copied!

namespace mge
{
    namespace
    {
        GLFWwindow *window = nullptr;

        VkInstance instance;
        VkSurfaceKHR surface;
#ifdef VK_ENABLE_VALIDATION
        constexpr bool enable_validation_layers = true;
        const std::vector<const char *> validation_layers = {"VK_LAYER_KHRONOS_validation"};
        VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
#endif
        VkDevice device;
        VkSwapchainKHR swapchain;

        void handle_key(GLFWwindow *window, int key, int, int action, int)
        {
#ifndef NDEBUG
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GLFW_TRUE);
#endif
        }

        void check_vk(VkResult result, const char *operation)
        {
            if (result != VK_SUCCESS)
                throw std::runtime_error(std::format("{} failed with error code {}", operation, (int)result));
        }

#ifdef VK_ENABLE_VALIDATION
        bool check_validation_layer_support()
        {
            uint32_t layer_count = 0;
            check_vk(vkEnumerateInstanceLayerProperties(&layer_count, nullptr), "vkEnumerateInstanceLayerProperties");

            std::vector<VkLayerProperties> available_layers(layer_count);
            check_vk(vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data()), "vkEnumerateInstanceLayerProperties");

            for (const char *requested_layer : validation_layers)
            {
                bool found = false;
                for (const VkLayerProperties &available_layer : available_layers)
                {
                    if (std::string(requested_layer) == available_layer.layerName)
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                    return false;
            }

            return true;
        }

        VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *)
        {
            const char *label = "INFO";
            if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
                label = "ERROR";
            else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                label = "WARNING";

            std::cerr << "[Vulkan " << label << "] " << callback_data->pMessage << '\n';
            return VK_FALSE;
        }

        VkResult create_debug_messenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *create_info, const VkAllocationCallbacks *allocator, VkDebugUtilsMessengerEXT *debug_messenger)
        {
            auto function = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

            if (function == nullptr)
            {
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }

            return function(instance, create_info, allocator, debug_messenger);
        }

        void destroy_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks *allocator)
        {
            if (debug_messenger == VK_NULL_HANDLE)
                return;

            auto function = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

            if (function != nullptr)
            {
                function(instance, debug_messenger, allocator);
            }
        }
#endif

        struct QueueFamilyIndices
        {
            std::optional<uint32_t> graphics;
            std::optional<uint32_t> present;

            bool complete() const
            {
                return graphics.has_value() && present.has_value();
            }
        };

        QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
        {
            uint32_t queue_family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

            std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

            QueueFamilyIndices indices;

            for (uint32_t index = 0; index < queue_family_count; ++index)
            {
                const VkQueueFamilyProperties &family = queue_families[index];

                if ((family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
                {
                    indices.graphics = index;
                }

                VkBool32 supports_present = VK_FALSE;
                check_vk(vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &supports_present), "vkGetPhysicalDeviceSurfaceSupportKHR");

                if (supports_present == VK_TRUE)
                {
                    indices.present = index;
                }

                if (indices.complete())
                {
                    break;
                }
            }

            return indices;
        }

        struct SwapchainSupportDetails
        {
            VkSurfaceCapabilitiesKHR capabilities{};
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> present_modes;
        };

        SwapchainSupportDetails query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface)
        {
            SwapchainSupportDetails details;

            check_vk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities), "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

            uint32_t format_count = 0;
            check_vk(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr), "vkGetPhysicalDeviceSurfaceFormatsKHR");

            if (format_count > 0)
            {
                details.formats.resize(format_count);
                check_vk(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data()), "vkGetPhysicalDeviceSurfaceFormatsKHR");
            }

            uint32_t present_mode_count = 0;
            check_vk(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr), "vkGetPhysicalDeviceSurfacePresentModesKHR");

            if (present_mode_count > 0)
            {
                details.present_modes.resize(present_mode_count);
                check_vk(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data()), "vkGetPhysicalDeviceSurfacePresentModesKHR");
            }

            return details;
        }

        VkSurfaceFormatKHR choose_surface_format(const std::vector<VkSurfaceFormatKHR> &formats)
        {
            for (const VkSurfaceFormatKHR &format : formats)
            {
                if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    return format;
                }
            }

            return formats.front();
        }

        VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR> &present_modes)
        {
            for (VkPresentModeKHR mode : present_modes)
            {
                if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    return mode;
                }
            }

            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkExtent2D choose_swapchain_extent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window)
        {
            if (capabilities.currentExtent.width !=
                std::numeric_limits<uint32_t>::max())
            {
                return capabilities.currentExtent;
            }

            int width = 0;
            int height = 0;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D extent{
                .width = static_cast<uint32_t>(width),
                .height = static_cast<uint32_t>(height),
            };

            extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return extent;
        }
    }

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

#ifdef VK_ENABLE_VALIDATION
        if (!check_validation_layer_support())
        {
            glfwTerminate();
            throw std::runtime_error("Validation layers are not installed. Install them with:\n  sudo pacman -S vulkan-validation-layers");
        }
#endif

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        GLFWwindow *window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);

        if (window == nullptr)
        {
            glfwTerminate();
            throw std::runtime_error("GLFW window creation failed.");
        }

        glfwSetKeyCallback(window, handle_key);

        uint32_t extension_count = 0;
        const char *const *glfw_extensions = glfwGetRequiredInstanceExtensions(&extension_count);

        if (glfw_extensions == nullptr)
        {
            glfwDestroyWindow(window);
            glfwTerminate();
            throw std::runtime_error("Could not retrieve GLFW Vulkan extensions.");
        }

        std::vector<const char *> extensions(glfw_extensions, glfw_extensions + extension_count);

#ifdef VK_ENABLE_VALIDATION
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        // Describes our application to the Vulkan runtime.
        VkApplicationInfo application_info{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = name.c_str(),
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "MaNieto's Game Engine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_3,
        };

        // Describes how the Vulkan instance should be created.
        VkInstanceCreateInfo instance_info{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &application_info,
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };

#ifdef VK_ENABLE_VALIDATION
        VkDebugUtilsMessengerCreateInfoEXT debug_create_info{
            .sType =
                VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debug_callback,
        };

        instance_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        instance_info.ppEnabledLayerNames = validation_layers.data();
        instance_info.pNext = &debug_create_info;
#endif

        instance = VK_NULL_HANDLE;

        VkResult result = vkCreateInstance(&instance_info, nullptr, &instance);
        if (result != VK_SUCCESS)
        {
            glfwDestroyWindow(window);
            glfwTerminate();
            throw std::runtime_error("Vulkan instance creation failed.");
        }

#ifdef VK_ENABLE_VALIDATION
        check_vk(create_debug_messenger(instance, &debug_create_info, nullptr, &debug_messenger), "create_debug_messenger");
#endif

#ifndef NDEBUG
        std::cout << "Vulkan instance created successfully.\n";
#endif

        surface = VK_NULL_HANDLE;
        check_vk(glfwCreateWindowSurface(instance, window, nullptr, &surface), "glfwCreateWindowSurface");

#ifndef NDEBUG
        std::cout << "Vulkan window surface created successfully.\n";
#endif

        // Ask Vulkan how many physical devices are available.
        uint32_t device_count = 0;
        check_vk(vkEnumeratePhysicalDevices(instance, &device_count, nullptr), "vkEnumeratePhysicalDevices");

        if (device_count == 0)
        {
            vkDestroySurfaceKHR(instance, surface, nullptr);
#ifdef VK_ENABLE_VALIDATION
            destroy_debug_messenger(instance, debug_messenger, nullptr);
#endif
            vkDestroyInstance(instance, nullptr);
            glfwDestroyWindow(window);
            glfwTerminate();
            throw std::runtime_error("No Vulkan-capable GPU was found.");
        }

        std::vector<VkPhysicalDevice> devices(device_count);

        check_vk(vkEnumeratePhysicalDevices(instance, &device_count, devices.data()), "vkEnumeratePhysicalDevices");

#ifndef NDEBUG
        std::cout << "Found " << device_count << " Vulkan device(s).\n";
#endif

        VkPhysicalDevice selected_device = VK_NULL_HANDLE;
        QueueFamilyIndices selected_queue_families;

        for (VkPhysicalDevice device : devices)
        {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(device, &properties);

#ifndef NDEBUG
            std::cout << "  " << properties.deviceName << '\n';
#endif

            QueueFamilyIndices queue_families = find_queue_families(device, surface);

            if (queue_families.complete() && selected_device == VK_NULL_HANDLE)
            {
                selected_device = device;
                selected_queue_families = queue_families;
            }
        }

        if (selected_device == VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(instance, surface, nullptr);
            vkDestroyInstance(instance, nullptr);
            glfwDestroyWindow(window);
            glfwTerminate();
            throw std::runtime_error("No GPU supports both graphics and presentation.");
        }

        const float queue_priority = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

        std::vector<uint32_t> unique_queue_families{*selected_queue_families.graphics};

        if (*selected_queue_families.present != *selected_queue_families.graphics)
        {
            unique_queue_families.push_back(*selected_queue_families.present);
        }

        for (uint32_t queue_family : unique_queue_families)
        {
            queue_create_infos.push_back(VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queue_family,
                .queueCount = 1,
                .pQueuePriorities = &queue_priority,
            });
        }

        const char *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        VkPhysicalDeviceFeatures device_features{};

        VkDeviceCreateInfo device_info{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
            .pQueueCreateInfos = queue_create_infos.data(),
            .enabledExtensionCount = 1,
            .ppEnabledExtensionNames = device_extensions,
            .pEnabledFeatures = &device_features,
        };

        device = VK_NULL_HANDLE;
        check_vk(vkCreateDevice(selected_device, &device_info, nullptr, &device), "vkCreateDevice");

        VkQueue graphics_queue = VK_NULL_HANDLE;
        VkQueue present_queue = VK_NULL_HANDLE;

        vkGetDeviceQueue(device, *selected_queue_families.graphics, 0, &graphics_queue);
        vkGetDeviceQueue(device, *selected_queue_families.present, 0, &present_queue);

        SwapchainSupportDetails swapchain_support = query_swapchain_support(selected_device, surface);

        if (swapchain_support.formats.empty() || swapchain_support.present_modes.empty())
        {
            vkDestroyDevice(device, nullptr);
            vkDestroySurfaceKHR(instance, surface, nullptr);
#ifdef VK_ENABLE_VALIDATION
            destroy_debug_messenger(instance, debug_messenger, nullptr);
#endif
            vkDestroyInstance(instance, nullptr);
            glfwDestroyWindow(window);
            glfwTerminate();
            throw std::runtime_error("Selected GPU has no usable swapchain support.");
        }

        VkSurfaceFormatKHR surface_format = choose_surface_format(swapchain_support.formats);
        VkPresentModeKHR present_mode = choose_present_mode(swapchain_support.present_modes);
        VkExtent2D swapchain_extent = choose_swapchain_extent(swapchain_support.capabilities, window);

        uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
        if (swapchain_support.capabilities.maxImageCount > 0 && image_count > swapchain_support.capabilities.maxImageCount)
        {
            image_count = swapchain_support.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapchain_info{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,
            .minImageCount = image_count,
            .imageFormat = surface_format.format,
            .imageColorSpace = surface_format.colorSpace,
            .imageExtent = swapchain_extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = swapchain_support.capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = present_mode,
            .clipped = VK_TRUE,
        };

        uint32_t queue_family_indices[] = {
            *selected_queue_families.graphics,
            *selected_queue_families.present,
        };

        if (*selected_queue_families.graphics != *selected_queue_families.present)
        {
            swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_info.queueFamilyIndexCount = 2;
            swapchain_info.pQueueFamilyIndices = queue_family_indices;
        }
        else
        {
            swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        swapchain = VK_NULL_HANDLE;
        check_vk(vkCreateSwapchainKHR(device, &swapchain_info, nullptr, &swapchain), "vkCreateSwapchainKHR");

        std::vector<VkImage> swapchain_images;
        check_vk(vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr), "vkGetSwapchainImagesKHR");
        swapchain_images.resize(image_count);
        check_vk(vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images.data()), "vkGetSwapchainImagesKHR");

        std::cout << "Swapchain created with " << swapchain_images.size() << " image(s), " << swapchain_extent.width << 'x' << swapchain_extent.height << ".\n";
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
        // Vulkan objects must be destroyed explicitly, in dependency order.
        // check_vk(vkDeviceWaitIdle(device), "vkDeviceWaitIdle");
        // vkDestroyFence(device, in_flight_fence, nullptr);
        // for (VkSemaphore semaphore : render_finished_semaphores)
        // {
        //     vkDestroySemaphore(device, semaphore, nullptr);
        // }
        // vkDestroySemaphore(device, image_available_semaphore, nullptr);
        // vkFreeCommandBuffers(device, command_pool, static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
        // vkDestroyCommandPool(device, command_pool, nullptr);
        // vkUnmapMemory(device, uniform_memory);
        // vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
        // vkDestroyBuffer(device, uniform_buffer, nullptr);
        // vkFreeMemory(device, uniform_memory, nullptr);
        // vkDestroyBuffer(device, index_buffer, nullptr);
        // vkFreeMemory(device, index_memory, nullptr);
        // vkDestroyBuffer(device, vertex_buffer, nullptr);
        // vkFreeMemory(device, vertex_memory, nullptr);
        // vkDestroyPipeline(device, graphics_pipeline, nullptr);
        // vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
        // vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
        // for (VkFramebuffer framebuffer : framebuffers)
        // {
        //     vkDestroyFramebuffer(device, framebuffer, nullptr);
        // }
        // vkDestroyRenderPass(device, render_pass, nullptr);
        // for (VkImageView image_view : swapchain_image_views)
        // {
        //     vkDestroyImageView(device, image_view, nullptr);
        // }
        vkDestroySwapchainKHR(device, swapchain, nullptr);
        vkDestroyDevice(device, nullptr);
#ifdef VK_ENABLE_VALIDATION
        destroy_debug_messenger(instance, debug_messenger, nullptr);
#endif
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        if (window != nullptr)
        {
            glfwDestroyWindow(window);
            window = nullptr;
        }

        glfwTerminate();
    }

    void Window::BeginFrame() {}

    void Window::EndFrame() {}

    void Window::Draw2D() {}

    void Window::Draw3D() {}

    void Window::SetBackgroundColor(float, float, float) {}
}
