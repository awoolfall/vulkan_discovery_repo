#pragma once

#include "vulkan_base.h"
#include <vulkan/vulkan.h>
#include <vector>

class graphics_pipeline
{
private:
    VkRenderPass render_pass;
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    void clear_shader_stages(vulkan_data& data);

protected:
    virtual VkPipelineVertexInputStateCreateInfo gen_vertex_input_info(vulkan_data& data) = 0;
    virtual VkPipelineInputAssemblyStateCreateInfo gen_input_assembly_info(vulkan_data& data);
    virtual std::vector<VkViewport> gen_viewport(vulkan_data& data);
    virtual std::vector<VkRect2D> gen_scissor(vulkan_data& data);
    virtual VkPipelineRasterizationStateCreateInfo gen_rasterization_state_info(vulkan_data& data);
    virtual VkPipelineMultisampleStateCreateInfo gen_multisampling_state_info(vulkan_data& data);
    virtual VkPipelineColorBlendStateCreateInfo gen_color_blend_state(vulkan_data& data, std::vector<VkPipelineColorBlendAttachmentState>& attachment_states);
    virtual std::vector<VkDynamicState> gen_dynamic_state_info(vulkan_data& data);
    virtual VkPipelineLayout gen_pipeline_layout(vulkan_data& data);
    virtual std::vector<VkPipelineShaderStageCreateInfo> load_shader_stage_infos(vulkan_data& data);

public:
    VkPipeline pipeline;
    VkPipelineLayout layout;

    void initialise(vulkan_data& data, VkRenderPass render_pass);
    void terminate(vulkan_data& data);
    VkPipeline get_pipeline() const;
    
};