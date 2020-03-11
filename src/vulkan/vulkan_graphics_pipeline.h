#pragma once

#include "vulkan_base.h"
#include "vulkan_uniform_buffer.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <tuple>

class graphics_pipeline
{
private:
    VkRenderPass render_pass;
    std::vector<uniform_buffer_base*> uniform_buffers;
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    void clear_shader_stages(vulkan_data& data);

protected:
    VkPipeline pipeline;
    VkPipelineLayout layout;
    VkDescriptorSetLayout descriptor_set_layout;

    static VkDescriptorSetLayoutBinding create_descriptor_set_binding(uint32_t binding, VkShaderStageFlags stages);

    /**
     * an abstract function where vertex input info is generated
     * @param data                      the vulkan context
     * @param binding_descriptions      a return vector to fill with binding descriptions
     * @param attrib_descriptions       a return vector to fill with attribute descriptions
     */
    virtual void gen_vertex_input_info(
            vulkan_data& data,
            std::vector<VkVertexInputBindingDescription>* binding_descriptions,
            std::vector<VkVertexInputAttributeDescription>* attrib_descriptions) = 0;
    /**
     * virtual function to define vulkan pipeline assembly state information
     * @param data the vulkan context
     * @return VkPipelineInputAssemblyStateCreateInfo
     */
    virtual VkPipelineInputAssemblyStateCreateInfo gen_input_assembly_info(vulkan_data& data);
    virtual std::vector<VkViewport> gen_viewport(vulkan_data& data);
    virtual std::vector<VkRect2D> gen_scissor(vulkan_data& data);
    virtual VkPipelineRasterizationStateCreateInfo gen_rasterization_state_info(vulkan_data& data);
    virtual VkPipelineMultisampleStateCreateInfo gen_multisampling_state_info(vulkan_data& data);
    virtual VkPipelineColorBlendStateCreateInfo gen_color_blend_state(vulkan_data& data, std::vector<VkPipelineColorBlendAttachmentState>& attachment_states);
    virtual std::vector<VkDynamicState> gen_dynamic_state_info(vulkan_data& data);
    virtual std::vector<VkPipelineShaderStageCreateInfo> load_shader_stage_infos(vulkan_data& data);
    virtual std::vector<VkDescriptorSetLayoutBinding> gen_descriptor_set_bindings(vulkan_data& data);

public:
    void initialise(vulkan_data& data, VkRenderPass input_render_pass);
    void terminate(vulkan_data& data);

    VkPipeline& get_pipeline();
    VkPipelineLayout& get_pipeline_layout();
    VkDescriptorSetLayout& get_descriptor_set_layout();
};

VkPipelineShaderStageCreateInfo gen_shader_stage_info_from_spirv(vulkan_data& data, std::string abs_path, shader_type type, const char* entry_point = "main");

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