#include "vulkan_command_buffer.h"

void graphics_command_buffer::initialise(vulkan_data& data)
{
    /* if it has already been initialised, then terminate to reinitialise */
    if (!this->command_buffers.empty()) {
        this->terminate(data);
    }
    this->reinitialise(data);
    register_command_buffer(data, this);
}

void graphics_command_buffer::reinitialise(vulkan_data& data)
{
    this->command_buffers.resize(data.swap_chain_data.frame_buffers.size());

    /* allocate command buffers */
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = data.command_pool_graphics;
    allocInfo.level = this->buffer_level;
    allocInfo.commandBufferCount = (uint32_t) this->command_buffers.size();

    if (vkAllocateCommandBuffers(data.logical_device, &allocInfo, this->command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    /* record the commands into the command buffers */
    for (size_t i = 0; i < this->command_buffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = this->flags; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(this->command_buffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        this->current_index = i;
        this->fill_command_buffer(data, i);

        if (vkEndCommandBuffer(this->command_buffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void graphics_command_buffer::terminate(vulkan_data& data)
{
    if (!command_buffers.empty()) {
        unregister_command_buffer(data, this);
        vkDeviceWaitIdle(data.logical_device);
        vkFreeCommandBuffers(data.logical_device, data.command_pool_graphics, (uint32_t)this->command_buffers.size(), this->command_buffers.data());
        this->command_buffers.clear();
    }
}

void graphics_command_buffer::reterminate(vulkan_data &data) {
    if (!command_buffers.empty()) {
        vkFreeCommandBuffers(data.logical_device, data.command_pool_graphics, (uint32_t)this->command_buffers.size(), this->command_buffers.data());
        this->command_buffers.clear();
    }
}
