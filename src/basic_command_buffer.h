#pragma once

#include <model/gltf_model.h>
#include "vulkan/vulkan_base.h"
#include "basic_pipeline.h"

struct render_obj {
    
};

class triangle_cmd : public graphics_command_buffer
{
private:

protected:
    void fill_command_buffer(vulkan_data& vkdata, size_t index) final;
    VkCommandBufferLevel get_buffer_level() const final;

public:
    basic_pipeline* pipeline = nullptr;
    buffer_base* vert_buffer = nullptr;
    buffer_base* index_buffer = nullptr;
    gltf_model* model = nullptr;
};
