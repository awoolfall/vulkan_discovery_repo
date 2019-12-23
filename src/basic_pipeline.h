#include "vulkan/vulkan_graphics_pipeline.h"

class basic_pipeline : public graphics_pipeline
{
protected:
    virtual VkPipelineVertexInputStateCreateInfo gen_vertex_input_info(vulkan_data& data) override;
    virtual std::vector<VkPipelineShaderStageCreateInfo> load_shader_stage_infos(vulkan_data& data) override;
};