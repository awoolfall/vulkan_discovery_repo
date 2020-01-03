#include "basic_pipeline.h"
#include "platform.h"

std::vector<VkVertexInputAttributeDescription> vertex::get_attribute_descriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    VkVertexInputAttributeDescription desc;
    
    desc.binding = 0;
    desc.location = 0;
    desc.format = VK_FORMAT_R32G32B32_SFLOAT;
    desc.offset = offsetof(vertex, pos);
    attributeDescriptions.push_back(desc);

    desc.binding = 0;
    desc.location = 1;
    desc.format = VK_FORMAT_R32G32B32_SFLOAT;
    desc.offset = offsetof(vertex, color);
    attributeDescriptions.push_back(desc);

    return attributeDescriptions;
}


void basic_pipeline::gen_vertex_input_info(vulkan_data& data, std::vector<VkVertexInputBindingDescription>* binding_descriptions, std::vector<VkVertexInputAttributeDescription>* attrib_descriptions)
{
    binding_descriptions->push_back(vertex::get_binding_description(0));
    *attrib_descriptions = vertex::get_attribute_descriptions();
}

std::vector<VkPipelineShaderStageCreateInfo> basic_pipeline::load_shader_stage_infos(vulkan_data& data)
{
    auto vert_info = gen_shader_stage_info_from_spirv(data, to_absolute_path("res/shaders/vertex_v.spv"), shader_type::VERTEX);
    auto frag_info = gen_shader_stage_info_from_spirv(data, to_absolute_path("res/shaders/vertex_f.spv"), shader_type::FRAGMENT);
    return {vert_info, frag_info};
}

std::vector<VkDynamicState> basic_pipeline::gen_dynamic_state_info(vulkan_data& data)
{
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_LINE_WIDTH
    };
    return dynamicStates;
}

std::vector<VkDescriptorSetLayout> basic_pipeline::gen_descriptor_set_layouts(vulkan_data& data)
{
    std::vector<VkDescriptorSetLayout> layouts;
    layouts.resize(1);
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(data.logical_device, &layoutInfo, nullptr, &layouts[0]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    return layouts;
}
