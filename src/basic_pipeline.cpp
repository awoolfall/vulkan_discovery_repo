#include "basic_pipeline.h"
#include "platform.h"

VkPipelineVertexInputStateCreateInfo basic_pipeline::gen_vertex_input_info(vulkan_data& data)
{
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional
    return vertexInputInfo;
}

std::vector<VkPipelineShaderStageCreateInfo> basic_pipeline::load_shader_stage_infos(vulkan_data& data)
{
    auto vert_data = read_data_from_binary_file(to_absolute_path("res/shaders/basic_v.spv"));
    auto frag_data = read_data_from_binary_file(to_absolute_path("res/shaders/basic_f.spv"));

    auto vert_module = create_shader_module_from_spirv(data, vert_data);
    auto frag_module = create_shader_module_from_spirv(data, frag_data);

    VkPipelineShaderStageCreateInfo vert_info = {};
    vert_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_info.module = vert_module;
    vert_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_info = {};
    frag_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_info.module = frag_module;
    frag_info.pName = "main";

    return {vert_info, frag_info};
}

std::vector<VkDynamicState> basic_pipeline::gen_dynamic_state_info(vulkan_data& data)
{
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_LINE_WIDTH
    };
    return dynamicStates;
}
