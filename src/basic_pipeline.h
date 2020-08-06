#pragma once

#include "vulkan/vulkan_base.h"
#include <glm/glm.hpp>

class basic_pipeline : public graphics_pipeline
{
public:
    struct vp_ubo
    {
        glm::mat4 view = glm::mat4(1.0);
        glm::mat4 proj = glm::mat4(1.0);
        glm::vec3 cameraPos = glm::vec3(0.0);
        float currTime = 0.0f;
    };

    struct m_ubo
    {
        glm::mat4 transform = glm::mat4(1.0);
    };

protected:
    void gen_vertex_input_info(vulkan_data& data,
            std::vector<VkVertexInputBindingDescription>* binding_descriptions,
            std::vector<VkVertexInputAttributeDescription>* attrib_descriptions) final;

    std::vector<VkPipelineShaderStageCreateInfo> load_shader_stage_infos(vulkan_data& data) final;
    std::vector<VkDynamicState> gen_dynamic_state_info(vulkan_data& data) final;
    std::vector<uniform_buffer_decl> get_uniform_buffer_declarations() final;
};
