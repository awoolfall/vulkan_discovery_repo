#pragma once

#include "vulkan_base.h"
#include "vulkan_uniform_buffer.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <tuple>

struct uniform_buffer_decl {
    uint32_t binding = 0;
    uniform_buffer_base* buffer = nullptr;
    VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    VkShaderStageFlagBits shaderFlags = VK_SHADER_STAGE_VERTEX_BIT;
};

class graphics_pipeline
{
private:
    VkRenderPass render_pass;
    std::vector<uniform_buffer_base*> uniform_buffers;
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    void clear_shader_stages(vulkan_data& data);

    std::vector<uniform_buffer_decl> uniformBufferDecls;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    void initialise_routine(vulkan_data& vkdata, VkRenderPass input_render_pass);
    void terminate_routine(vulkan_data& vkdata);

protected:
    VkPipeline pipeline;
    VkPipelineLayout layout;


    static VkDescriptorSetLayoutBinding create_descriptor_set_binding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags stages);

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
    virtual std::vector<uniform_buffer_decl> get_uniform_buffer_declarations();

public:
    void initialise(vulkan_data& vkdata, VkRenderPass input_render_pass);
    void terminate(vulkan_data& data);
    void reinitialise(vulkan_data& vkdata, VkRenderPass input_render_pass);
    void reterminate(vulkan_data& vkdata);

    void populate_descriptor_sets(vulkan_data& vkdata);

    VkPipeline& get_pipeline(vulkan_data& vkdata);
    VkPipelineLayout& get_pipeline_layout();
    VkDescriptorSet& get_descriptor_set(size_t index);
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

inline uniform_buffer_decl new_uniform_buffer_decl(uint32_t binding, VkDescriptorType type, uniform_buffer_base* buffer, VkShaderStageFlagBits shaderFlags) {
    uniform_buffer_decl decl;
    decl.binding = binding;
    decl.type = type;
    decl.buffer = buffer;
    decl.shaderFlags = shaderFlags;
    return decl;
}

#define VERTEX_INPUT_DESCRIPTIONS(type) \
    static VkVertexInputBindingDescription get_binding_description(size_t binding){ return ::get_binding_description<type>(binding); } \
    static VkVertexInputBindingDescription get_binding_description_instanced(size_t binding){ return ::get_binding_description_instanced<type>(binding); } \
    static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();
