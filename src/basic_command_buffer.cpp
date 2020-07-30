#include "basic_command_buffer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <array>

VkCommandBufferLevel triangle_cmd::get_buffer_level() const {
    return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
}

void triangle_cmd::virtual_terminate(vulkan_data& vkdata) 
{
    for (auto& buf_set : this->m_uniform_buffers) {
        for (auto& buf : buf_set) {
            buf.terminate(vkdata);
        }
    }
    for (auto& buf : this->vp_uniform_buffers) {
        buf.terminate(vkdata);
    }
    for (auto& buf : this->color_uniform_buffers) {
        buf.terminate(vkdata);
    }
}

void triangle_cmd::fill_command_buffer(vulkan_data& vkdata, size_t index)
{

    // ready model uniform buffers for this frame
    if (index == this->m_uniform_buffers.size()) {
        this->m_uniform_buffers.emplace_back();
    } else if (index < this->m_uniform_buffers.size()) {
        for (auto& buf : this->m_uniform_buffers[index]) {
            buf.terminate(vkdata);
        }
        this->m_uniform_buffers[index].clear();
    } else {
        this->m_uniform_buffers.resize(index+1);
    }

    // fill vp uniform buffers
    if (index == this->vp_uniform_buffers.size()) {
        this->vp_uniform_buffers.emplace_back();
        this->vp_uniform_buffers[index].initialise(vkdata, 0, this->pipeline->get_descriptor_set_layout(0));
    } else if (index > this->vp_uniform_buffers.size()) {
        throw std::runtime_error("...");
    }

    this->vp_uniform_buffers[index].data().view = this->view_matrix;
    this->vp_uniform_buffers[index].data().proj = this->projection_matrix;
    this->vp_uniform_buffers[index].update_buffer(vkdata);

    // fill color buffers
    if (this->color_uniform_buffers.empty()) {
        this->color_uniform_buffers.resize(this->model->vk_image_data().size()+1);

        this->color_uniform_buffers[0].image_view().initialise(vkdata, vkdata.default_image->image);
        this->color_uniform_buffers[0].sampler().initialise(vkdata);
        this->color_uniform_buffers[0].initialise(vkdata, 2, this->pipeline->get_descriptor_set_layout(2));

        for (size_t i = 1; i < this->color_uniform_buffers.size(); i++) {
            this->color_uniform_buffers[i].image_view().initialise(vkdata, this->model->vk_image_data()[i-1].image);
            this->color_uniform_buffers[i].sampler().initialise(vkdata);
            this->color_uniform_buffers[i].initialise(vkdata, 2, this->pipeline->get_descriptor_set_layout(2));
        }
    }

    // fill command buffer
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

    for (auto n : this->model->model().scenes.front().nodes) {
        this->rec_fill_command_buffer_model(vkdata, index, this->model->model().nodes[n], glm::mat4(1.0f));
    }

    vkCmdEndRenderPass(cmd_buffer());
}

void triangle_cmd::rec_fill_command_buffer_model(vulkan_data& vkdata, const size_t& index, const tinygltf::Node &current_node, glm::mat4 transform) {
    // modify to combine this nodes transform with parent's transform
    if (!current_node.matrix.empty()) {
        transform += glm::mat4({
            current_node.matrix[0], current_node.matrix[1], current_node.matrix[2], current_node.matrix[3],
            current_node.matrix[4], current_node.matrix[5], current_node.matrix[6], current_node.matrix[7],
            current_node.matrix[8], current_node.matrix[9], current_node.matrix[10], current_node.matrix[11],
            current_node.matrix[12], current_node.matrix[13], current_node.matrix[14], current_node.matrix[15]
        });
    } else {
        if (!current_node.scale.empty()) {
            transform = glm::scale(transform, {(float)current_node.scale[0], (float)current_node.scale[1], (float)current_node.scale[2]});
        }
        if (!current_node.rotation.empty()) {
            transform = transform * glm::mat4_cast(glm::quat((float)current_node.rotation[0], (float)current_node.rotation[1], (float)current_node.rotation[2], (float)current_node.rotation[3]));
        }
        if (!current_node.translation.empty()) {
            transform = glm::translate(transform, {(float)current_node.translation[0], (float)current_node.translation[1], (float)current_node.translation[2]});
        }
    }

    // do render commands if necessary
    if (current_node.mesh >= 0) {
        // create m uniform buffer
        auto& ubo = this->m_uniform_buffers[index].emplace_back();
        ubo.data().transform = transform;
        ubo.initialise(vkdata, 1, this->pipeline->get_descriptor_set_layout(1));
        ubo.update_buffer(vkdata);

        // @TODO: deal with meshes with more than 1 primitive
        const auto& primitive_data = this->model->vk_mesh_data()[current_node.mesh].primitive_data.front();

        // record render commands
        vkCmdBindPipeline(cmd_buffer(),
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          this->pipeline->get_pipeline(vkdata));

        int color_tex = primitive_data.color_tex;
        if (color_tex < -1)
            color_tex = -1;

        std::array<VkDescriptorSet, 3> descriptor_sets = {
            this->vp_uniform_buffers[index].get_descriptor_set(index),
            ubo.get_descriptor_set(index),
            this->color_uniform_buffers[color_tex+1].get_descriptor_set(index)
        };

        vkCmdBindDescriptorSets(cmd_buffer(),
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                this->pipeline->get_pipeline_layout(),
                                0, static_cast<uint32_t>(descriptor_sets.size()),
                                descriptor_sets.data(),
                                0, nullptr);

        VkBuffer vert_buffers[] = {primitive_data.vertex_buffer.get_vk_buffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd_buffer(), 0, 1, vert_buffers, offsets);

        // determine whether the mesh is indexed or not and draw accordingly
        if (primitive_data.has_index_buffer) {
            vkCmdBindIndexBuffer(cmd_buffer(), primitive_data.index_buffer.get_vk_buffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmd_buffer(), static_cast<uint32_t>(primitive_data.index_buffer.get_count()), 1, 0, 0, 0);
        } else {
            vkCmdDraw(cmd_buffer(), static_cast<uint32_t>(primitive_data.vertex_buffer.get_count()), 1, 0, 0);
        }
    }

    // do for all node children with new transform
    for (int c : current_node.children) {
        rec_fill_command_buffer_model(vkdata, index, this->model->model().nodes[c], transform);
    }
}
