#pragma once

#include "vulkan_base.h"

template <typename T>
class vertex_buffer
{
public:
    VkBuffer buffer;
    VmaAllocation allocation;

    void initialise(vulkan_data& data, std::vector<T> vertex_data)
    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(vertex_data[0]) * vertex_data.size();
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        if (vmaCreateBuffer(data.mem_allocator, &bufferInfo, &allocInfo, &this->buffer, &this->allocation, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("failed to create vertex buffer!");
        }

        void* mapped_data;
        vmaMapMemory(data.mem_allocator, this->allocation, &mapped_data);
        memcpy(mapped_data, vertex_data.data(), vertex_data.size() * sizeof(vertex_data[0]));
        vmaUnmapMemory(data.mem_allocator, this->allocation);
    }

    void terminate(vulkan_data& data)
    {
        vkDeviceWaitIdle(data.logical_device);
        vmaDestroyBuffer(data.mem_allocator, this->buffer, this->allocation);
    }

};
