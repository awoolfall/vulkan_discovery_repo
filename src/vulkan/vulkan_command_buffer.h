#pragma once
#include "vulkan_graphics.h"
#include "vulkan_base.h"
#include <vulkan/vulkan.h>

class graphics_command_buffer
{
private:
    std::vector<VkCommandBuffer> command_buffers;
    uint32_t current_index = 0;

public:
    uint32_t flags = 0x00;
    VkCommandBufferLevel buffer_level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    void initialise(vulkan_data& data);
    void reinitialise(vulkan_data& data);
    void reterminate(vulkan_data& data);
    void terminate(vulkan_data& data);
    std::vector<VkCommandBuffer>& cmd_buffers() {return command_buffers;}

protected:
    virtual void fill_command_buffer(vulkan_data& data, size_t index) = 0;

    inline VkCommandBuffer& cmd_buffer() {
        return command_buffers[current_index];
    }

    inline VkFramebuffer& frame_buffer(vulkan_data& data) {
        return data.swap_chain_data.frame_buffers[current_index];
    }
    
};