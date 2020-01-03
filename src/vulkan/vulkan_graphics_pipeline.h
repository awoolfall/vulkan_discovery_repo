#pragma once

#include "vulkan_base.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <tuple>

class graphics_pipeline
{
private:
    VkRenderPass render_pass;
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    void clear_shader_stages(vulkan_data& data);

protected:
    virtual void gen_vertex_input_info(vulkan_data& data, std::vector<VkVertexInputBindingDescription>* binding_descriptions, std::vector<VkVertexInputAttributeDescription>* attrib_descriptions) = 0;
    virtual VkPipelineInputAssemblyStateCreateInfo gen_input_assembly_info(vulkan_data& data);
    virtual std::vector<VkViewport> gen_viewport(vulkan_data& data);
    virtual std::vector<VkRect2D> gen_scissor(vulkan_data& data);
    virtual VkPipelineRasterizationStateCreateInfo gen_rasterization_state_info(vulkan_data& data);
    virtual VkPipelineMultisampleStateCreateInfo gen_multisampling_state_info(vulkan_data& data);
    virtual VkPipelineColorBlendStateCreateInfo gen_color_blend_state(vulkan_data& data, std::vector<VkPipelineColorBlendAttachmentState>& attachment_states);
    virtual std::vector<VkDynamicState> gen_dynamic_state_info(vulkan_data& data);
    virtual std::vector<VkDescriptorSetLayout> gen_descriptor_set_layouts(vulkan_data& data);
    virtual VkPipelineLayout gen_pipeline_layout(vulkan_data& data);
    virtual std::vector<VkPipelineShaderStageCreateInfo> load_shader_stage_infos(vulkan_data& data);

public:
    VkPipeline pipeline;
    VkPipelineLayout layout;
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts;

    void initialise(vulkan_data& data, VkRenderPass render_pass);
    void terminate(vulkan_data& data);
    VkPipeline get_pipeline() const;
    
};

VkPipelineShaderStageCreateInfo gen_shader_stage_info_from_spirv(vulkan_data& data, std::string& abs_path, shader_type type, const char* entry_point = "main");

template <typename T>
VkVertexInputBindingDescription get_binding_description(size_t binding)
{
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = (uint32_t)binding;
    bindingDescription.stride = sizeof(T);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

template <typename T>
VkVertexInputBindingDescription get_binding_description_instanced(size_t binding)
{
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = (uint32_t)binding;
    bindingDescription.stride = sizeof(T);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
    return bindingDescription;
}

#define VERTEX_INPUT_DESCRIPTIONS(type) static VkVertexInputBindingDescription get_binding_description(size_t binding){ return ::get_binding_description<type>(binding); } static VkVertexInputBindingDescription get_binding_description_instanced(size_t binding){ return ::get_binding_description_instanced<type>(binding); } static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();