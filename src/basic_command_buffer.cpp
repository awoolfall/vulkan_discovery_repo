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

    // not the best solution. models may contain repeating nodes in a scene
    for (auto& node : model->model().nodes) {
        if (node.mesh >= 0) {
            vkCmdBindPipeline(cmd_buffer(),
                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                              this->pipeline.get_pipeline(vkdata));

            vkCmdBindDescriptorSets(cmd_buffer(),
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        this->pipeline.get_pipeline_layout(),
                        0, 1,
                        &node.pipeline.get_descriptor_set(index),
                        0, nullptr);

            // @TODO: deal with meshes with more than 1 primitive
            auto& primitive_data = model->vk_mesh_data()[node.mesh].primitive_data.front();

            VkBuffer vert_buffers[] = {primitive_data.vertex_buffer.get_vk_buffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(cmd_buffer(), 0, 1, vert_buffers, offsets);

            // determine whether the mesh is indexed or not and draw accordingly
            if (primitive_data.has_index_buffer) {
                vkCmdBindIndexBuffer(cmd_buffer(), primitive_data.index_buffer.get_vk_buffer(), 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(cmd_buffer(), primitive_data.index_buffer.get_count(), 1, 0, 0, 0);
            } else {
                vkCmdDraw(cmd_buffer(), primitive_data.vertex_buffer.get_count(), 1, 0, 0);
            }
        }
    }

    vkCmdEndRenderPass(cmd_buffer());
}
