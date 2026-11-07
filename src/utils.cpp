#include "utils.h"

namespace mge
{
    GLFWwindow *window = nullptr;

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
} // namespace mge
