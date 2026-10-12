#include "mge/window.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "utils.h"

#include <format>
#include <iostream>
#include <stdexcept>
#include <vector>

// #define VK_ENABLE_VALIDATION // Destroy once all the code is copied!

namespace mge
{
    namespace
    {
        VkInstance instance = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkExtent2D swapchain_extent;
#ifdef VK_ENABLE_VALIDATION
        constexpr bool enable_validation_layers = true;
        const std::vector<const char *> validation_layers = {"VK_LAYER_KHRONOS_validation"};
        VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
#endif
        VkDevice device = VK_NULL_HANDLE;
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        std::vector<VkImageView> swapchain_image_views;
        VkRenderPass render_pass = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> swapchain_frame_buffers;
        VkCommandPool command_pool = VK_NULL_HANDLE;
        VkCommandBuffer command_buffer = VK_NULL_HANDLE;
        VkSemaphore image_available_semaphore = VK_NULL_HANDLE;
        std::vector<VkSemaphore> render_finished_semaphores;
        VkFence in_flight_fence = VK_NULL_HANDLE;
        uint32_t image_index = 0;
        VkQueue graphics_queue = VK_NULL_HANDLE;
        VkQueue present_queue = VK_NULL_HANDLE;
        VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

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

        bool supports_device_extensions(VkPhysicalDevice device)
        {
            uint32_t extension_count = 0;
            check_vk(vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr), "vkEnumerateDeviceExtensionProperties");

            std::vector<VkExtensionProperties> available_extensions(extension_count);
            check_vk(vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data()), "vkEnumerateDeviceExtensionProperties");

            for (const VkExtensionProperties &extension : available_extensions)
            {
                if (std::string(extension.extensionName) == VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                    return true;
            }

            return false;
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

        void record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index)
        {
            VkCommandBufferBeginInfo begin_info{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = 0,
                .pInheritanceInfo = nullptr,
            };

            check_vk(vkBeginCommandBuffer(command_buffer, &begin_info), "vkBeginCommandBuffer");

            VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

            VkRenderPassBeginInfo render_pass_info{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .renderPass = render_pass,
                .framebuffer = swapchain_frame_buffers[image_index],
                .renderArea{
                    .offset = {0, 0},
                    .extent = swapchain_extent,
                },
                .clearValueCount = 1,
                .pClearValues = &clear_color,
            };

            vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdEndRenderPass(command_buffer);
            check_vk(vkEndCommandBuffer(command_buffer), "vkEndCommandBuffer");
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

        window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);

        if (window == nullptr)
        {
            glfwTerminate();
            throw std::runtime_error("GLFW window creation failed.");
        }

        try
        {
            uint32_t extension_count = 0;
            const char *const *glfw_extensions = glfwGetRequiredInstanceExtensions(&extension_count);

            if (glfw_extensions == nullptr)
                throw std::runtime_error("Could not retrieve GLFW Vulkan extensions.");

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
                throw std::runtime_error("Vulkan instance creation failed.");

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
                throw std::runtime_error("No Vulkan-capable GPU was found.");

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

                if (!supports_device_extensions(device))
                    continue;

                QueueFamilyIndices queue_families = find_queue_families(device, surface);

                if (queue_families.complete() && selected_device == VK_NULL_HANDLE)
                {
                    selected_device = device;
                    selected_queue_families = queue_families;
                }
            }

            if (selected_device == VK_NULL_HANDLE)
                throw std::runtime_error("No GPU supports both graphics and presentation.");

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

            graphics_queue = VK_NULL_HANDLE;
            present_queue = VK_NULL_HANDLE;

            vkGetDeviceQueue(device, *selected_queue_families.graphics, 0, &graphics_queue);
            vkGetDeviceQueue(device, *selected_queue_families.present, 0, &present_queue);

            SwapchainSupportDetails swapchain_support = query_swapchain_support(selected_device, surface);

            if (swapchain_support.formats.empty() || swapchain_support.present_modes.empty())
                throw std::runtime_error("Selected GPU has no usable swapchain support.");

            VkSurfaceFormatKHR surface_format = choose_surface_format(swapchain_support.formats);
            VkPresentModeKHR present_mode = choose_present_mode(swapchain_support.present_modes);
            swapchain_extent = choose_swapchain_extent(swapchain_support.capabilities, window);

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

#ifndef NDEBUG
            std::cout << "Swapchain created with " << swapchain_images.size() << " image(s), " << swapchain_extent.width << 'x' << swapchain_extent.height << ".\n";
#endif

            swapchain_image_views.reserve(swapchain_images.size());

            for (VkImage image : swapchain_images)
            {
                VkImageViewCreateInfo image_view_info{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image = image,
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .format = surface_format.format,
                    .components = VkComponentMapping{
                        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                    },
                    .subresourceRange = VkImageSubresourceRange{
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    },
                };

                VkImageView image_view = VK_NULL_HANDLE;
                check_vk(vkCreateImageView(device, &image_view_info, nullptr, &image_view), "vkCreateImageView");

                swapchain_image_views.push_back(image_view);
            }

#ifndef NDEBUG
            std::cout << "Created " << swapchain_image_views.size() << " swapchain image view(s).\n";
#endif

            VkAttachmentDescription color_attachment{
                .format = surface_format.format,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            };

            VkAttachmentReference color_attachment_reference{
                .attachment = 0,
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            };

            VkSubpassDescription subpass{
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .colorAttachmentCount = 1,
                .pColorAttachments = &color_attachment_reference,
            };

            VkSubpassDependency dependency{
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            };

            VkRenderPassCreateInfo render_pass_info{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .attachmentCount = 1,
                .pAttachments = &color_attachment,
                .subpassCount = 1,
                .pSubpasses = &subpass,
                .dependencyCount = 1,
                .pDependencies = &dependency,
            };

            render_pass = VK_NULL_HANDLE;
            check_vk(vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass), "vkCreateRenderPass");

#ifndef NDEBUG
            std::cout << "Render pass created successfully.\n";
#endif
            swapchain_frame_buffers.resize(swapchain_image_views.size());

            for (int i = 0; i < swapchain_frame_buffers.size(); ++i)
            {
                VkImageView attachments[] = {swapchain_image_views[i]};

                VkFramebufferCreateInfo framebuffer_info{
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .renderPass = render_pass,
                    .attachmentCount = 1,
                    .pAttachments = attachments,
                    .width = swapchain_extent.width,
                    .height = swapchain_extent.height,
                    .layers = 1,
                };

                check_vk(vkCreateFramebuffer(device, &framebuffer_info, nullptr, &swapchain_frame_buffers[i]), "vkCreateFramebuffer");
#ifndef NDEBUG
                std::cout << "Framebuffer created successfully.\n";
#endif
            }

            VkCommandPoolCreateInfo pool_info{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = *selected_queue_families.graphics,
            };

            check_vk(vkCreateCommandPool(device, &pool_info, nullptr, &command_pool), "vkCreateCommandPool");

#ifndef NDEBUG
            std::cout << "Command pool created successfully.\n";
#endif
            VkCommandBufferAllocateInfo allocate_info{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = command_pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };

            check_vk(vkAllocateCommandBuffers(device, &allocate_info, &command_buffer), "vkAllocateCommandBuffers");

#ifndef NDEBUG
            std::cout << "Command buffers allocated successfully.\n";
#endif
            VkSemaphoreCreateInfo semaphore_info{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };

            VkFenceCreateInfo fence_info{
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT,
            };

            check_vk(vkCreateSemaphore(device, &semaphore_info, nullptr, &image_available_semaphore), "vkCreateSemaphore");

            render_finished_semaphores = std::vector<VkSemaphore>(swapchain_images.size(), VK_NULL_HANDLE);

            for (VkSemaphore &semaphore : render_finished_semaphores)
            {
                check_vk(vkCreateSemaphore(device, &semaphore_info, nullptr, &semaphore), "vkCreateSemaphore");
            }

            check_vk(vkCreateFence(device, &fence_info, nullptr, &in_flight_fence), "vkCreateFence");

#ifndef NDEBUG
            std::cout << "Semaphores and fence created successfully.\n";
#endif
        }
        catch (...)
        {
            Window::Destroy();
            throw;
        }
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
        if (device != VK_NULL_HANDLE)
        {
            vkDeviceWaitIdle(device);

            if (in_flight_fence != VK_NULL_HANDLE)
                vkDestroyFence(device, in_flight_fence, nullptr);

            for (VkSemaphore semaphore : render_finished_semaphores)
            {
                if (semaphore != VK_NULL_HANDLE)
                    vkDestroySemaphore(device, semaphore, nullptr);
            }
            if (image_available_semaphore != VK_NULL_HANDLE)
                vkDestroySemaphore(device, image_available_semaphore, nullptr);

            if (command_pool != VK_NULL_HANDLE)
                vkDestroyCommandPool(device, command_pool, nullptr);

            for (VkFramebuffer framebuffer : swapchain_frame_buffers)
            {
                if (framebuffer != VK_NULL_HANDLE)
                    vkDestroyFramebuffer(device, framebuffer, nullptr);
            }

            if (render_pass != VK_NULL_HANDLE)
                vkDestroyRenderPass(device, render_pass, nullptr);

            for (VkImageView image_view : swapchain_image_views)
            {
                if (image_view != VK_NULL_HANDLE)
                    vkDestroyImageView(device, image_view, nullptr);
            }

            if (swapchain != VK_NULL_HANDLE)
                vkDestroySwapchainKHR(device, swapchain, nullptr);

            vkDestroyDevice(device, nullptr);
        }

        command_pool = VK_NULL_HANDLE;
        swapchain_frame_buffers.clear();
        swapchain_image_views.clear();
        render_pass = VK_NULL_HANDLE;
        swapchain = VK_NULL_HANDLE;
        device = VK_NULL_HANDLE;
#ifdef VK_ENABLE_VALIDATION
        destroy_debug_messenger(instance, debug_messenger, nullptr);
        debug_messenger = VK_NULL_HANDLE;
#endif
        if (instance != VK_NULL_HANDLE)
        {
            if (surface != VK_NULL_HANDLE)
                vkDestroySurfaceKHR(instance, surface, nullptr);
            vkDestroyInstance(instance, nullptr);
        }

        surface = VK_NULL_HANDLE;
        instance = VK_NULL_HANDLE;

        if (window != nullptr)
        {
            glfwDestroyWindow(window);
            window = nullptr;
        }

        glfwTerminate();
    }

    void Window::BeginFrame()
    {
        check_vk(vkWaitForFences(device, 1, &in_flight_fence, VK_TRUE, UINT64_MAX), "vkWaitForFences");

        image_index = 0;
        VkResult acquire_result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, image_available_semaphore, VK_NULL_HANDLE, &image_index);

        if (acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR)
        {
            check_vk(acquire_result, "vkAcquireNextImageKHR");
        }

        check_vk(vkResetFences(device, 1, &in_flight_fence), "vkResetFences");

        check_vk(vkResetCommandBuffer(command_buffer, 0), "vkResetCommandBuffer");

        VkCommandBufferBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        };
        check_vk(vkBeginCommandBuffer(command_buffer, &begin_info), "vkBeginCommandBuffer");

        VkRenderPassBeginInfo render_pass_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = render_pass,
            .framebuffer = swapchain_frame_buffers[image_index],
            .renderArea{
                .offset = {0, 0},
                .extent = swapchain_extent,
            },
            .clearValueCount = 1,
            .pClearValues = &clear_color,
        };

        vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void Window::EndFrame()
    {
        vkCmdEndRenderPass(command_buffer);

        check_vk(vkEndCommandBuffer(command_buffer), "vkEndCommandBuffer");

        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submit_info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &image_available_semaphore,
            .pWaitDstStageMask = &wait_stage,
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &render_finished_semaphores[image_index],
        };

        check_vk(vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fence), "vkQueueSubmit");

        VkPresentInfoKHR present_info{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores =
                &render_finished_semaphores[image_index],
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &image_index,
        };

        VkResult present_result = vkQueuePresentKHR(present_queue, &present_info);
        if (present_result != VK_SUCCESS && present_result != VK_SUBOPTIMAL_KHR)
        {
            check_vk(present_result, "vkQueuePresentKHR");
        }
    }

    void Window::Draw2D() {}

    void Window::Draw3D() {}

    void Window::SetBackgroundColor(float r, float g, float b)
    {
        clear_color = {{{r, g, b, 1.0f}}};
    }
}
