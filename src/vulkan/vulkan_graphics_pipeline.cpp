#include "vulkan_graphics_pipeline.h"

#include <utility>
#include "../platform.h"

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

std::vector<VkPipelineShaderStageCreateInfo> graphics_pipeline::load_shader_stage_infos(vulkan_data& data)
{
    throw std::logic_error("Have not passed any shader stages and no default shader loading has been set!");
    std::vector<VkPipelineShaderStageCreateInfo> stages;
    return stages;
}

void graphics_pipeline::initialise(vulkan_data& vkdata, VkRenderPass input_render_pass)
{
    this->uniformBufferDecls = get_uniform_buffer_declarations();
    // initialise uniform buffers
    for (uniform_buffer_decl &decl : this->uniformBufferDecls) {
        decl.buffer->initialise(vkdata, decl.binding);
    }
    this->initialise_routine(vkdata, input_render_pass);

    // register to vkdata
    register_pipeline(vkdata, this);
}

void graphics_pipeline::clear_shader_stages(vulkan_data& data)
{
    // destroy all generated shader modules
    for (VkPipelineShaderStageCreateInfo stage : this->shader_stages) {
        vkDestroyShaderModule(data.logical_device, stage.module, nullptr);
    }
    this->shader_stages.clear();
}

void graphics_pipeline::terminate(vulkan_data& data)
{
    unregister_pipeline(data, this);
    terminate_routine(data);

    // terminate all uniform buffers
    for (uniform_buffer_decl &decl : this->uniformBufferDecls) {
        decl.buffer->terminate(data);
    }
}

VkPipeline &graphics_pipeline::get_pipeline() {
    return pipeline;
}

VkPipelineLayout &graphics_pipeline::get_pipeline_layout() {
    return layout;
}

VkDescriptorSetLayoutBinding graphics_pipeline::create_descriptor_set_binding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags stages) {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = binding;
    uboLayoutBinding.descriptorType = descriptor_type;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = stages;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
    return uboLayoutBinding;
}

std::vector<uniform_buffer_decl> graphics_pipeline::get_uniform_buffer_declarations() {
    return {};
}

void graphics_pipeline::populate_descriptor_sets(vulkan_data& vkdata) {
    // return early if there are no descriptors
    if (this->uniformBufferDecls.empty()) { return; }

    // check if we actually need to repopulate the descriptor sets
    bool shouldRepopulate = false;
    for (uniform_buffer_decl& decl : this->uniformBufferDecls) {
        if (decl.buffer->should_repopulate_descriptor_sets()) {
            shouldRepopulate = true;
        }
    }
    if (!shouldRepopulate) { return; }

    // create writeDescriptorSets and update
    for (size_t i = 0; i < vkdata.swap_chain_data.images.size(); i++) {
        std::vector<uniform_info> uniformInfos;
        uniformInfos.reserve(this->uniformBufferDecls.size());
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        descriptorWrites.reserve(this->uniformBufferDecls.size());

        for (uniform_buffer_decl& decl : this->uniformBufferDecls) {
            uniformInfos.push_back(decl.buffer->get_uniform_info(i));
            auto& info = uniformInfos.back();

            VkWriteDescriptorSet descriptorWrite = {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = this->descriptorSets[i];
            descriptorWrite.dstBinding = decl.binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorCount = 1;

            switch (info.type) {
                case uniform_info::UNIFORM_BUFFER:
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descriptorWrite.pBufferInfo = &info.bufferInfo;
                    break;
                case uniform_info::SAMPLER_BUFFER:
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrite.pImageInfo = &info.imageInfo;
                    break;
            }
            descriptorWrites.push_back(descriptorWrite);
        }

        vkUpdateDescriptorSets(vkdata.logical_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

VkDescriptorSet &graphics_pipeline::get_descriptor_set(size_t index) {
    return this->descriptorSets[index];
}

void graphics_pipeline::recreate(vulkan_data &vkdata, VkRenderPass input_render_pass) {
    this->terminate_routine(vkdata);
    this->initialise_routine(vkdata, input_render_pass);
}

void graphics_pipeline::initialise_routine(vulkan_data &vkdata, VkRenderPass input_render_pass) {
    std::vector<VkVertexInputBindingDescription> binding_descriptions; std::vector<VkVertexInputAttributeDescription> attrib_descriptions;
    this->gen_vertex_input_info(vkdata, &binding_descriptions, &attrib_descriptions);

    VkPipelineVertexInputStateCreateInfo vertex_input_state_info = {};
    vertex_input_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_info.vertexBindingDescriptionCount = (uint32_t)binding_descriptions.size();
    vertex_input_state_info.pVertexBindingDescriptions = binding_descriptions.data();
    vertex_input_state_info.vertexAttributeDescriptionCount = (uint32_t)attrib_descriptions.size();
    vertex_input_state_info.pVertexAttributeDescriptions = attrib_descriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = this->gen_input_assembly_info(vkdata);
    std::vector<VkViewport> viewports = this->gen_viewport(vkdata);
    std::vector<VkRect2D> scissors = this->gen_scissor(vkdata);
    VkPipelineRasterizationStateCreateInfo rasterization_state_info = this->gen_rasterization_state_info(vkdata);
    VkPipelineMultisampleStateCreateInfo multisample_state_info = this->gen_multisampling_state_info(vkdata);
    std::vector<VkPipelineColorBlendAttachmentState> attachment_states;
    VkPipelineColorBlendStateCreateInfo color_blend_state_info = this->gen_color_blend_state(vkdata, attachment_states);
    std::vector<VkDynamicState> dynamic_states = this->gen_dynamic_state_info(vkdata);

    if (this->shader_stages.empty()) {
        this->shader_stages = this->load_shader_stage_infos(vkdata);
    }

    // create the descriptor set layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    if (!this->uniformBufferDecls.empty()) {
        // create layout bindings
        std::vector<VkDescriptorSetLayoutBinding> layout_bindings;
        for (uniform_buffer_decl& decl : this->uniformBufferDecls) {
            layout_bindings.push_back(create_descriptor_set_binding(decl.binding, decl.type, decl.shaderFlags));
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layout_bindings.size());
        layoutInfo.pBindings = layout_bindings.data();
        if (vkCreateDescriptorSetLayout(vkdata.logical_device, &layoutInfo, nullptr, &this->descriptor_set_layout) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        // generate descriptor pool sizes
        std::vector<VkDescriptorPoolSize> poolSizes;
        auto num_swap_chain_images = static_cast<uint32_t>(vkdata.swap_chain_data.images.size());
        for (uniform_buffer_decl& decl : this->uniformBufferDecls) {
            VkDescriptorPoolSize poolSize{};
            poolSize.type = decl.type;
            poolSize.descriptorCount = num_swap_chain_images;
            poolSizes.push_back(poolSize);
        }

        // create descriptor pool
        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = num_swap_chain_images;

        if (vkCreateDescriptorPool(vkdata.logical_device, &poolInfo, nullptr, &this->descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }

        // create descriptor sets
        std::vector<VkDescriptorSetLayout> layouts(num_swap_chain_images, this->descriptor_set_layout);
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = this->descriptorPool;
        allocInfo.descriptorSetCount = num_swap_chain_images;
        allocInfo.pSetLayouts = layouts.data();

        this->descriptorSets.resize(num_swap_chain_images);
        if (vkAllocateDescriptorSets(vkdata.logical_device, &allocInfo, this->descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        this->populate_descriptor_sets(vkdata);

        // generate pipeline layout
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &this->descriptor_set_layout;
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
    } else {
        // generate pipeline layout
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
    }

    if (vkCreatePipelineLayout(vkdata.logical_device, &pipelineLayoutInfo, nullptr, &this->layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    this->render_pass = input_render_pass;

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
    pipelineInfo.stageCount = (uint32_t)this->shader_stages.size();
    pipelineInfo.pStages = this->shader_stages.data();
    pipelineInfo.pVertexInputState = &vertex_input_state_info;
    pipelineInfo.pInputAssemblyState = &input_assembly_info;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterization_state_info;
    pipelineInfo.pMultisampleState = &multisample_state_info;
    pipelineInfo.pDepthStencilState = nullptr; // Optional @TODO
    pipelineInfo.pColorBlendState = &color_blend_state_info;
    if (dynamic_states.empty()) {
        pipelineInfo.pDynamicState = nullptr;
    } else {
        pipelineInfo.pDynamicState = &dynamic_state_info;
    }
    pipelineInfo.layout = this->layout;
    pipelineInfo.renderPass = input_render_pass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    auto err = vkCreateGraphicsPipelines(vkdata.logical_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->pipeline);
    if (err != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

void graphics_pipeline::terminate_routine(vulkan_data &vkdata) {
    vkDestroyPipeline(vkdata.logical_device, this->pipeline, nullptr);
    vkDestroyPipelineLayout(vkdata.logical_device, this->layout, nullptr);
    vkDestroyDescriptorPool(vkdata.logical_device, this->descriptorPool, nullptr);

    this->clear_shader_stages(vkdata);
}

VkPipelineShaderStageCreateInfo gen_shader_stage_info_from_spirv(vulkan_data& data, std::string abs_path, shader_type type, const char* entry_point)
{
    auto vert_data = read_data_from_binary_file(std::move(abs_path));
    auto module = create_shader_module_from_spirv(data, vert_data);
    auto info = gen_shader_stage_create_info(module, type, entry_point);
    return info;
}
