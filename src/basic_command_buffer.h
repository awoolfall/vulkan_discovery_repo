#pragma once

#include "vulkan/vulkan_command_buffer.h"

class triangle_cmd : public graphics_command_buffer
{
protected:
    void fill_command_buffer(vulkan_data& data, size_t index) final;

public:
    graphics_pipeline* pipeline = nullptr;
    buffer_base* vert_buffer = nullptr;
    buffer_base* index_buffer = nullptr;
    uniform_buffer_base* uniform_buffers = nullptr;
};
