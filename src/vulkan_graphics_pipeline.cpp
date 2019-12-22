#include "vulkan_graphics_pipeline.h"


VkPipelineInputAssemblyStateCreateInfo graphics_pipeline::gen_input_assembly_info(vulkan_data& data)
{
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    return inputAssembly;
}

std::vector<VkViewport> graphics_pipeline::gen_viewport(vulkan_data& data)
{

}

std::vector<VkRect2D> graphics_pipeline::gen_scissor(vulkan_data& data)
{

}

VkPipelineRasterizationStateCreateInfo graphics_pipeline::gen_rasterization_state_info(vulkan_data& data)
{

}

VkPipelineMultisampleStateCreateInfo graphics_pipeline::gen_multisampling_state_info(vulkan_data& data)
{

}

VkPipelineColorBlendStateCreateInfo graphics_pipeline::gen_color_blend_state(vulkan_data& data, std::vector<VkPipelineColorBlendAttachmentState>& attachment_states)
{

}

std::vector<VkDynamicState> graphics_pipeline::gen_dynamic_state_info(vulkan_data& data)
{

}

VkPipelineLayout graphics_pipeline::gen_pipeline_layout(vulkan_data& data)
{

}

VkRenderPass graphics_pipeline::gen_render_pass(vulkan_data& data)
{

}

std::vector<VkPipelineShaderStageCreateInfo> graphics_pipeline::load_shader_stage_infos(vulkan_data& data)
{
    std::vector<VkPipelineShaderStageCreateInfo> stages;
    return stages;
}


void graphics_pipeline::initialise(vulkan_data& data, std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {})
{
    VkPipelineVertexInputStateCreateInfo vertex_input_state_info = this->gen_vertex_input_info(data);
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = this->gen_input_assembly_info(data);
    std::vector<VkViewport> viewports = this->gen_viewport(data);
    std::vector<VkRect2D> scissors = this->gen_scissor(data);
    VkPipelineRasterizationStateCreateInfo rasterization_state_info = this->gen_rasterization_state_info(data);
    VkPipelineMultisampleStateCreateInfo multisample_state_info = this->gen_multisampling_state_info(data);
    std::vector<VkPipelineColorBlendAttachmentState> attachment_states;
    VkPipelineColorBlendStateCreateInfo color_blend_state_info = this->gen_color_blend_state(data, attachment_states);
    std::vector<VkDynamicState> dynamic_states = this->gen_dynamic_state_info(data);
    VkRenderPass render_pass = this->gen_render_pass(data);
    std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos = this->load_shader_stage_infos(data);
    this->layout = this->gen_pipeline_layout(data);

    VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
    dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_info.dynamicStateCount = dynamic_states.size();
    dynamic_state_info.pDynamicStates = dynamic_states.data();

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = viewports.size();
    viewportState.pViewports = viewports.data();
    viewportState.scissorCount = scissors.size();
    viewportState.pScissors = scissors.data();

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shader_stage_infos.size();
    pipelineInfo.pStages = shader_stage_infos.data();
    pipelineInfo.pVertexInputState = &vertex_input_state_info;
    pipelineInfo.pInputAssemblyState = &input_assembly_info;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterization_state_info;
    pipelineInfo.pMultisampleState = &multisample_state_info;
    pipelineInfo.pDepthStencilState = nullptr; // Optional @TODO
    pipelineInfo.pColorBlendState = &color_blend_state_info;
    if (dynamic_states.size() == 0) {
        pipelineInfo.pDynamicState = nullptr;
    } else {
        pipelineInfo.pDynamicState = &dynamic_state_info;
    }
    pipelineInfo.layout = this->layout;
    pipelineInfo.renderPass = render_pass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(data.logical_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

void graphics_pipeline::terminate(vulkan_data& data)
{
    vkDestroyPipeline(data.logical_device, this->pipeline, nullptr);
    vkDestroyPipelineLayout(data.logical_device, this->layout, nullptr);
}

VkPipeline graphics_pipeline::get_pipeline() const
{

}
