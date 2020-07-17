#pragma once

#include <model/model_3d.h>
#include "vulkan/vulkan_command_buffer.h"
#include "basic_pipeline.h"

struct render_obj {
    
};

class triangle_cmd : public graphics_command_buffer
{
protected:
    void fill_command_buffer(vulkan_data& vkdata, size_t index) final;
    VkCommandBufferLevel get_buffer_level() const final;

public:
    basic_pipeline* pipeline = nullptr;
    buffer_base* vert_buffer = nullptr;
    buffer_base* index_buffer = nullptr;
    model_3d* model = nullptr;
};
