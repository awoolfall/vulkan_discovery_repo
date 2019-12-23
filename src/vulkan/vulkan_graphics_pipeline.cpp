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
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) data.swap_chain_data.extent.width;
    viewport.height = (float) data.swap_chain_data.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    std::vector<VkViewport> viewports;
    viewports.push_back(viewport);
    return viewports;
}

std::vector<VkRect2D> graphics_pipeline::gen_scissor(vulkan_data& data)
{
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = data.swap_chain_data.extent;

    std::vector<VkRect2D> scissors;
    scissors.push_back(scissor);
    return scissors;
}

VkPipelineRasterizationStateCreateInfo graphics_pipeline::gen_rasterization_state_info(vulkan_data& data)
{
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    return rasterizer;
}

VkPipelineMultisampleStateCreateInfo graphics_pipeline::gen_multisampling_state_info(vulkan_data& data)
{
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    return multisampling;
}

VkPipelineColorBlendStateCreateInfo graphics_pipeline::gen_color_blend_state(vulkan_data& data, std::vector<VkPipelineColorBlendAttachmentState>& attachment_states)
{
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    attachment_states.push_back(colorBlendAttachment);

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = (uint32_t)attachment_states.size();
    colorBlending.pAttachments = attachment_states.data();
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    return colorBlending;
}

std::vector<VkDynamicState> graphics_pipeline::gen_dynamic_state_info(vulkan_data& data)
{
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };
    return dynamicStates;
}

VkPipelineLayout graphics_pipeline::gen_pipeline_layout(vulkan_data& data)
{
    VkPipelineLayout pipelineLayout;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(data.logical_device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
    return pipelineLayout;
}

VkRenderPass graphics_pipeline::gen_render_pass(vulkan_data& data)
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = data.swap_chain_data.image_format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass = {};  
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkRenderPass render_pass;
    if (vkCreateRenderPass(data.logical_device, &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
    return render_pass;
}

std::vector<VkPipelineShaderStageCreateInfo> graphics_pipeline::load_shader_stage_infos(vulkan_data& data)
{
    throw std::logic_error("Have not passed any shader stages and no default shader loading has been set!");
    std::vector<VkPipelineShaderStageCreateInfo> stages;
    return stages;
}


void graphics_pipeline::initialise(vulkan_data& data, std::vector<VkPipelineShaderStageCreateInfo> shader_stages)
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
    this->render_pass = this->gen_render_pass(data);
    std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos = this->load_shader_stage_infos(data);
    this->layout = this->gen_pipeline_layout(data);

    VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
    dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_info.dynamicStateCount = (uint32_t)dynamic_states.size();
    dynamic_state_info.pDynamicStates = dynamic_states.data();

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = (uint32_t)viewports.size();
    viewportState.pViewports = viewports.data();
    viewportState.scissorCount = (uint32_t)scissors.size();
    viewportState.pScissors = scissors.data();

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = (uint32_t)shader_stage_infos.size();
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

    // destroy all generated shader modules
    if (shader_stages.size() == 0) {
        for (VkPipelineShaderStageCreateInfo stage : shader_stage_infos) {
            vkDestroyShaderModule(data.logical_device, stage.module, nullptr);
        }
    }
}

void graphics_pipeline::terminate(vulkan_data& data)
{
    vkDestroyRenderPass(data.logical_device, this->render_pass, nullptr);
    vkDestroyPipeline(data.logical_device, this->pipeline, nullptr);
    vkDestroyPipelineLayout(data.logical_device, this->layout, nullptr);
}

VkPipeline graphics_pipeline::get_pipeline() const
{
    return this->pipeline;
}
