#pragma once

#include "vulkan/vulkan_graphics_pipeline.h"
#include <glm/glm.hpp>

class basic_pipeline : public graphics_pipeline
{
public:
    struct vertex
    {
        glm::vec3 pos = glm::vec3(0.0);
        glm::vec3 color = glm::vec3(0.0);

        VERTEX_INPUT_DESCRIPTIONS(vertex);
    };

    struct mvp_ubo
    {
        glm::mat4 model = glm::mat4(0.0);
        glm::mat4 view = glm::mat4(0.0);
        glm::mat4 proj = glm::mat4(0.0);
    };

    inline uniform_buffer<mvp_ubo> new_mvp_ubo(vulkan_data& data) {
        uniform_buffer<mvp_ubo> buf;
        buf.initialise(data, 0, this->get_descriptor_set_layout());
        return buf;
    }

protected:
    void gen_vertex_input_info(vulkan_data& data,
            std::vector<VkVertexInputBindingDescription>* binding_descriptions,
            std::vector<VkVertexInputAttributeDescription>* attrib_descriptions) final;

    std::vector<VkPipelineShaderStageCreateInfo> load_shader_stage_infos(vulkan_data& data) final;
    std::vector<VkDynamicState> gen_dynamic_state_info(vulkan_data& data) final;
    std::vector<VkDescriptorSetLayoutBinding> gen_descriptor_set_bindings(vulkan_data &data) final;
};
