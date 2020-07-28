#pragma once

#include <model/gltf_model.h>
#include "vulkan/vulkan_base.h"
#include "basic_pipeline.h"

class triangle_cmd : public graphics_command_buffer
{
private:
    std::vector<uniform_buffer<basic_pipeline::vp_ubo>> vp_uniform_buffers;
    std::vector<std::vector<uniform_buffer<basic_pipeline::m_ubo>>> m_uniform_buffers;
    std::vector<sampler_uniform_buffer> color_uniform_buffers;

    void rec_fill_command_buffer_model(vulkan_data& vkdata, const size_t& index, const tinygltf::Node& current_node, glm::mat4 parent_transform);

protected:
    VkCommandBufferLevel get_buffer_level() const final;
    void virtual_terminate(vulkan_data& vkdata) final;

public:
    basic_pipeline* pipeline = nullptr;
    gltf_model* model = nullptr;

    glm::mat4 projection_matrix = glm::mat4(1.0);
    glm::mat4 view_matrix = glm::mat4(1.0);

    // @TODO: terminate all buffers
    void preterminate(vulkan_data& data);
    void fill_command_buffer(vulkan_data& vkdata, size_t index) final;

};
