#ifndef MGE_COMPILED_UTILS_H
#define MGE_COMPILED_UTILS_H

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <vector>

#define VK_ENABLE_VALIDATION // Comment to disable validation

namespace mge
{
    extern GLFWwindow *window;

    extern VkInstance instance;
    extern VkSurfaceKHR surface;
    extern VkExtent2D swapchain_extent;
#ifdef VK_ENABLE_VALIDATION
    inline constexpr bool enable_validation_layers = true;
    extern const std::vector<const char *> validation_layers;
    extern VkDebugUtilsMessengerEXT debug_messenger;
#endif
    extern VkDevice device;
    extern VkSwapchainKHR swapchain;
    extern std::vector<VkImageView> swapchain_image_views;
    extern VkRenderPass render_pass;
    extern std::vector<VkFramebuffer> swapchain_frame_buffers;
    extern VkCommandPool command_pool;
    extern VkCommandBuffer command_buffer;
    extern VkSemaphore image_available_semaphore;
    extern std::vector<VkSemaphore> render_finished_semaphores;
    extern VkFence in_flight_fence;
    extern uint32_t image_index;
    extern VkQueue graphics_queue;
    extern VkQueue present_queue;
} // mge

#endif