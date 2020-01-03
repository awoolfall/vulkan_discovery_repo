#pragma once

#include "vulkan/vulkan_graphics_pipeline.h"
#include <glm/glm.hpp>

struct vertex
{
    glm::vec3 pos;
    glm::vec3 color;

    VERTEX_INPUT_DESCRIPTIONS(vertex);
};

class basic_pipeline : public graphics_pipeline
{
protected:
    virtual void gen_vertex_input_info(vulkan_data& data, std::vector<VkVertexInputBindingDescription>* binding_descriptions, std::vector<VkVertexInputAttributeDescription>* attrib_descriptions) override;
    virtual std::vector<VkPipelineShaderStageCreateInfo> load_shader_stage_infos(vulkan_data& data) override;
    virtual std::vector<VkDynamicState> gen_dynamic_state_info(vulkan_data& data) override;
    virtual std::vector<VkDescriptorSetLayout> gen_descriptor_set_layouts(vulkan_data& data) override;
};
