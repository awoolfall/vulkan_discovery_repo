#pragma once

#include "vulkan/vulkan_command_buffer.h"

class triangle_cmd : public graphics_command_buffer
{
protected:
    virtual void fill_command_buffer(vulkan_data& data, VkCommandBuffer& buffer, VkFramebuffer& frame_buffer) override final;

public:
    VkPipeline* pipeline;
    VkBuffer* vert_buffer;
};
