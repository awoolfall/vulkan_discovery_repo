#pragma once

#include "vulkan_base.h"

class buffer_base
{
public:
    virtual VkBuffer& get_vk_buffer() = 0;

};

template <typename T>
class static_buffer : public buffer_base
{
private:
    VkBuffer buffer{};
    VmaAllocation allocation{};

    static void copy_buffer(vulkan_data& data, VkBuffer src_buffer, VkBuffer dst_buffer, size_t data_size)
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = data.command_pool_graphics;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer cmd;
        vkAllocateCommandBuffers(data.logical_device, &allocInfo, &cmd);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmd, &beginInfo);
        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = data_size;
        vkCmdCopyBuffer(cmd, src_buffer, dst_buffer, 1, &copyRegion);
        vkEndCommandBuffer(cmd);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;

        vkQueueSubmit(data.graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(data.graphics_queue);

        vkFreeCommandBuffers(data.logical_device, data.command_pool_graphics, 1, &cmd);
    }

public:
    void initialise(vulkan_data& data, VkBufferUsageFlagBits usage, std::vector<T> vertex_data)
    {
        size_t byte_size = (vertex_data.size() * sizeof(T));
        create_buffer(data, &buffer, &allocation, byte_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VMA_MEMORY_USAGE_GPU_ONLY);
        VkBuffer staging_buffer; VmaAllocation staging_allocation;
        create_buffer(data, &staging_buffer, &staging_allocation, byte_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
        fill_buffer(data, staging_allocation, vertex_data);
        copy_buffer(data, staging_buffer, buffer, byte_size);
        vmaDestroyBuffer(data.mem_allocator, staging_buffer, staging_allocation);
    }

    void terminate(vulkan_data& data)
    {
        vkDeviceWaitIdle(data.logical_device);
        vmaDestroyBuffer(data.mem_allocator, buffer, allocation);
    }

    VkBuffer& get_vk_buffer() final
    {
        return buffer;
    }
};

template <typename T>
class dynamic_buffer : public buffer_base
{
private:
    VkBuffer buffer{};
    VmaAllocation allocation{};

public:
    void initialise(vulkan_data& data, VkBufferUsageFlagBits usage, size_t data_length)
    {
        create_buffer(data, &buffer, &allocation, data_length, usage, VMA_MEMORY_USAGE_CPU_TO_GPU);
    }

    void fill_buffer(vulkan_data& vkdata, std::vector<T> data)
    {
        ::fill_buffer(vkdata, allocation, data);
    }

    void terminate(vulkan_data& data)
    {
        vkDeviceWaitIdle(data.logical_device);
        vmaDestroyBuffer(data.mem_allocator, buffer, allocation);
    }

    VkBuffer& get_vk_buffer() final
    {
        return buffer;
    }
};
