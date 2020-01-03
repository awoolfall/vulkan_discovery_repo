#pragma once
#include "vulkan_base.h"

template <typename T>
class uniform_buffer
{
private:
    std::vector<VkBuffer> uniform_buffers;
    std::vector<VmaAllocation> allocations;

public:
    void initialise(vulkan_data& data)
    {
        VkDeviceSize buffer_size = sizeof(T);
        size_t num_swap_chain_images = data.swap_chain_data.images.size();
        uniform_buffers.resize(num_swap_chain_images);
        allocations.resize(num_swap_chain_images);

        for (size_t i = 0; i < num_swap_chain_images; i++) {
            ::create_buffer(data, &uniform_buffers[i], &allocations[i], buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        }
    }

    void terminate(vulkan_data& data)
    {
        vkDeviceWaitIdle(data.logical_device);
        for (size_t i = 0; i < num_swap_chain_images; i++) {
            vmaDestroyBuffer(data.mem_allocator, uniform_buffers[i], allocations[i]);
        }
    }

    void fill_buffer(vulkan_data& data, T& ubo)
    {
        uint32_t image_index = ::get_image_index(data);

        void* data;
        vmaMapMemory(data.mem_allocator, allocations[image_index], &data)
        memcpy(data, &ubo, sizeof(T));
        vmaUnmapMemory(data.mem_allocator, allocations[image_index]);
    }
};