#include "vulkan_base.h"

void cmd_copy_data_to_buffer(vulkan_data& data, VkBuffer src_buffer, VkBuffer dst_buffer, size_t data_size)
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


void buffer_base::initialise_static(vulkan_data& data, VkBufferUsageFlagBits usage, void* input_data, size_t byte_size)
{
    this->is_static = true;
    this->max_byte_size = byte_size;
    ::create_buffer(data, &buffer, &allocation, byte_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VMA_MEMORY_USAGE_GPU_ONLY);
    VkBuffer staging_buffer; VmaAllocation staging_allocation;
    ::create_buffer(data, &staging_buffer, &staging_allocation, byte_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    ::fill_buffer(data, staging_allocation, byte_size, input_data);
    cmd_copy_data_to_buffer(data, staging_buffer, buffer, byte_size);
    vmaDestroyBuffer(data.mem_allocator, staging_buffer, staging_allocation);
}

void buffer_base::initialise_dynamic(vulkan_data& data, VkBufferUsageFlagBits usage, size_t byte_size)
{
    this->is_static = false;
    this->max_byte_size = byte_size;
    ::create_buffer(data, &buffer, &allocation, byte_size, usage, VMA_MEMORY_USAGE_CPU_TO_GPU);
}

void buffer_base::terminate(vulkan_data& data)
{
    vkDeviceWaitIdle(data.logical_device);
    vmaDestroyBuffer(data.mem_allocator, buffer, allocation);
}

bool buffer_base::fill_buffer(vulkan_data& vkdata, void* data, size_t byte_size)
{
    if (this->is_static) {
        return false;
    } else {
        if (byte_size == this->max_byte_size) {
            ::fill_buffer(vkdata, allocation, this->max_byte_size, data);
            return true;
        }
    }
    return false;
}

VkBuffer buffer_base::get_vk_buffer() const {
    return this->buffer;
}

size_t buffer_base::byte_size() const {
    return this->max_byte_size;
}
