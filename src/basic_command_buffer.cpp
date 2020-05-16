#include "basic_command_buffer.h"

VkCommandBufferLevel triangle_cmd::get_buffer_level() const {
    return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
}

void triangle_cmd::fill_command_buffer(vulkan_data& vkdata, size_t index)
{
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.11f, 0.12f, 0.15f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vkdata.render_pass;
    renderPassInfo.framebuffer = frame_buffer(vkdata);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = vkdata.swap_chain_data.extent;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(cmd_buffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    for (auto& node : model->nodes) {
        if (node.has_mesh()) {
            vkCmdBindPipeline(cmd_buffer(),
                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                              node.pipeline.get_pipeline(vkdata));

            VkBuffer vert_buffers[] = {model->meshes[node.mesh_index].vertex_buffer.get_vk_buffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(cmd_buffer(), 0, 1, vert_buffers, offsets);
            vkCmdBindIndexBuffer(cmd_buffer(), model->meshes[node.mesh_index].index_buffer.get_vk_buffer(), 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets(cmd_buffer(),
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    node.pipeline.get_pipeline_layout(),
                                    0, 1,
                                    &node.pipeline.get_descriptor_set(index),
                                    0, nullptr);

            vkCmdDrawIndexed(cmd_buffer(), model->meshes[node.mesh_index].index_buffer.get_count(), 1, 0, 0, 0);
        }
    }

//    vkCmdBindPipeline(cmd_buffer(),
//                      VK_PIPELINE_BIND_POINT_GRAPHICS,
//                      this->pipeline->get_pipeline(vkdata));
//
//    VkBuffer vert_buffers[] = {this->vert_buffer->get_vk_buffer()};
//    VkBuffer index_buffers[] = {this->index_buffer->get_vk_buffer()};
//    VkDeviceSize offsets[] = {0};
//    vkCmdBindVertexBuffers(cmd_buffer(), 0, 1, vert_buffers, offsets);
//    vkCmdBindIndexBuffer(cmd_buffer(), this->index_buffer->get_vk_buffer(), 0, VK_INDEX_TYPE_UINT32);
//
//    vkCmdBindDescriptorSets(cmd_buffer(),
//                            VK_PIPELINE_BIND_POINT_GRAPHICS,
//                            this->pipeline->get_pipeline_layout(),
//                            0, 1,
//                            &this->pipeline->get_descriptor_set(index),
//                            0, nullptr);
//
//    vkCmdDrawIndexed(cmd_buffer(), this->index_buffer->get_count(), 1, 0, 0, 0);

    vkCmdEndRenderPass(cmd_buffer());
}
