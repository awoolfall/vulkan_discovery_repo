#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <array>
#include "vk_mem_alloc.h"

class graphics_command_buffer;
class graphics_pipeline;

#define MAX_FRAMES_IN_FLIGHT 2
struct vulkan_data
{
    GLFWwindow* glfw_window;
    VkInstance instance = nullptr;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice logical_device = VK_NULL_HANDLE;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkSwapchainKHR swap_chain;
    VkRenderPass render_pass;
    VkCommandPool command_pool_graphics;
    struct {
        std::vector<VkImage> images;
        std::vector<VkImageView> image_views;
        std::vector<VkFramebuffer> frame_buffers;
        VkFormat image_format;
        VkExtent2D extent;
    } swap_chain_data;
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> image_available_sems;
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> render_finished_sems;
    std::array<VkFence, MAX_FRAMES_IN_FLIGHT> in_flight_fences;
    int32_t image_index = -1;
    uint32_t current_frame = 0;
    std::vector<graphics_command_buffer*> registered_command_buffers;
    std::vector<graphics_pipeline*> registered_pipelines;
    VmaAllocator mem_allocator;
};

enum class shader_type {
    VERTEX, FRAGMENT, GEOMETRY, COMPUTE
};


void initialise_vulkan(vulkan_data* data, GLFWwindow* window);
void terminate_vulkan(vulkan_data& data);
bool is_vulkan_initialised(vulkan_data& data);

VkShaderModule create_shader_module_from_spirv(vulkan_data& vulkan, std::vector<char>& shader_data);
VkPipelineShaderStageCreateInfo gen_shader_stage_create_info(VkShaderModule module, shader_type type, const char* entry_point = "main");

void submit_command_buffers_graphics(vulkan_data& data, std::vector<VkCommandBuffer> command_buffers);
void present_frame(vulkan_data& data);

void register_command_buffer(vulkan_data& data, graphics_command_buffer* buffer);
void unregister_command_buffer(vulkan_data& data, graphics_command_buffer* buffer);
void register_pipeline(vulkan_data& data, graphics_pipeline* pipeline);
void unregister_pipeline(vulkan_data& data, graphics_pipeline* pipeline);

void create_buffer(vulkan_data& data, VkBuffer* buffer, VmaAllocation* allocation, size_t byte_data_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage);
uint32_t get_image_index(vulkan_data& data);

template <typename T>
void fill_buffer(vulkan_data& data, VmaAllocation& alloc, std::vector<T>& buffer_data)
{
    size_t data_size = buffer_data.size() * sizeof(buffer_data[0]);
    void* mapped_data;
    vmaMapMemory(data.mem_allocator, alloc, &mapped_data);
    memcpy(mapped_data, buffer_data.data(), data_size);
    vmaUnmapMemory(data.mem_allocator, alloc);
}