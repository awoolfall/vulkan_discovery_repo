#include "basic_pipeline.h"
#include "platform.h"
#include "model/gltf_model.h"

void basic_pipeline::gen_vertex_input_info(
        vulkan_data& data,
        std::vector<VkVertexInputBindingDescription>* binding_descriptions,
        std::vector<VkVertexInputAttributeDescription>* attrib_descriptions)
{
    binding_descriptions->push_back(vertex::get_binding_description(0));
    *attrib_descriptions = vertex::get_attribute_descriptions();
}

std::vector<VkPipelineShaderStageCreateInfo> basic_pipeline::load_shader_stage_infos(vulkan_data& data)
{
    auto vert_info = gen_shader_stage_info_from_spirv(data,
            to_absolute_path("res/shaders/vertex_v.spv"),
            shader_type::VERTEX);
    auto frag_info = gen_shader_stage_info_from_spirv(data,
            to_absolute_path("res/shaders/vertex_f.spv"),
            shader_type::FRAGMENT);
    return {vert_info, frag_info};
}

std::vector<VkDynamicState> basic_pipeline::gen_dynamic_state_info(vulkan_data& data)
{
    return {
        VK_DYNAMIC_STATE_LINE_WIDTH
    };
}

std::vector<uniform_buffer_decl> basic_pipeline::get_uniform_buffer_declarations() {
    return {
        new_uniform_buffer_decl(0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
        new_uniform_buffer_decl(1, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT),
        new_uniform_buffer_decl(2, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
        new_uniform_buffer_decl(3, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
    };
}
