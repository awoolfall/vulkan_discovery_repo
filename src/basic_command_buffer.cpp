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

    vkCmdBindPipeline(cmd_buffer(),
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      this->pipeline->get_pipeline());

    VkBuffer vert_buffers[] = {this->vert_buffer->get_vk_buffer()};
    VkBuffer index_buffers[] = {this->index_buffer->get_vk_buffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd_buffer(), 0, 1, vert_buffers, offsets);
    vkCmdBindIndexBuffer(cmd_buffer(), this->index_buffer->get_vk_buffer(), 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(cmd_buffer(),
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            this->pipeline->get_pipeline_layout(),
                            0, 1,
                            &this->pipeline->get_descriptor_set(index),
                            0, nullptr);

    vkCmdDrawIndexed(cmd_buffer(), this->index_buffer->get_count(), 1, 0, 0, 0);

    vkCmdEndRenderPass(cmd_buffer());
}
