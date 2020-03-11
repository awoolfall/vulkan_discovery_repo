#include "basic_command_buffer.h"

void triangle_cmd::fill_command_buffer(vulkan_data& data, size_t index)
{
    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = data.render_pass;
    renderPassInfo.framebuffer = frame_buffer(data);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = data.swap_chain_data.extent;
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(cmd_buffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindDescriptorSets(cmd_buffer(),
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            this->pipeline->get_pipeline_layout(),
                            0, 1,
                            &this->uniform_buffers->get_descriptor_sets()[index],
                            0, nullptr);
    vkCmdBindPipeline(cmd_buffer(),
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            this->pipeline->get_pipeline());

    VkBuffer vert_buffers[] = {this->vert_buffer->get_vk_buffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd_buffer(), 0, 1, vert_buffers, offsets);
    
    vkCmdDraw(cmd_buffer(), 3, 1, 0, 0);

    vkCmdEndRenderPass(cmd_buffer());
}
