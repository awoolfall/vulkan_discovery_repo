#include "basic_command_buffer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
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
    for (auto& buf : this->sampler_buffers) {
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

    this->vp_uniform_buffers[index].data() = this->frame_ubo;
    this->vp_uniform_buffers[index].update_buffer(vkdata);

    // fill color buffers
    if (this->sampler_buffers.empty()) {
        this->sampler_buffers.push_back({}); // fill in default texs
        this->sampler_buffers.back().image_view().initialise(vkdata, *vkdata.default_image);
        this->sampler_buffers.back().sampler().initialise(vkdata);
        this->sampler_buffers.back().initialise(vkdata, 0, this->pipeline->get_descriptor_set_layout(2));

        this->sampler_buffers.push_back({}); // fill in default texs
        this->sampler_buffers.back().image_view().initialise(vkdata, *vkdata.default_image);
        this->sampler_buffers.back().sampler().initialise(vkdata);
        this->sampler_buffers.back().initialise(vkdata, 0, this->pipeline->get_descriptor_set_layout(3));
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

    int scene_to_load = this->model->model().defaultScene;
    for (auto n : this->model->model().scenes[scene_to_load].nodes) {
        this->rec_fill_command_buffer_model(vkdata, index, this->model->model().nodes[n], glm::mat4(1.0f));
    }

    vkCmdEndRenderPass(cmd_buffer());
}

void triangle_cmd::rec_fill_command_buffer_model(vulkan_data &vkdata, const size_t &index, const tinygltf::Node &current_node, glm::mat4 transform) {
    // modify to combine this nodes transform with parent's transform
    glm::mat4 this_transform = glm::mat4(1.0f);
    if (!current_node.matrix.empty()) {
        this_transform = glm::make_mat4x4(current_node.matrix.data());
    } else {
        if (!current_node.translation.empty()) {
            this_transform = glm::translate(this_transform, glm::vec3(glm::make_vec3(current_node.translation.data())));
        }
        if (!current_node.rotation.empty()) {
            glm::quat q = glm::make_quat(current_node.rotation.data());
            this_transform = this_transform * glm::mat4(q);
        }
        if (!current_node.scale.empty()) {
            this_transform = glm::scale(this_transform, glm::vec3(glm::make_vec3(current_node.scale.data())));
        }
    }
    transform = transform * this_transform;

    // do render commands if necessary
    if (current_node.mesh >= 0) {
        // @TODO: deal with meshes with more than 1 primitive
        const auto& primitive_data = this->model->vk_mesh_data()[current_node.mesh].primitive_data.front();

        /* tex data */
        // color tex
        int color_tex = primitive_data.tex_indexes.color;
        if (color_tex >= 0) {
            if (this->sampler_tex_map.count(color_tex) == 0) {
                // tex does not yet exist as descriptor, create it
                this->sampler_tex_map.emplace(color_tex, static_cast<int>(this->sampler_buffers.size()));
                this->sampler_buffers.push_back({});
                this->sampler_buffers.back().image_view().initialise(vkdata, this->model->vk_image_data()[color_tex]);
                this->sampler_buffers.back().sampler().initialise(vkdata);
                this->sampler_buffers.back().initialise(vkdata, 0, this->pipeline->get_descriptor_set_layout(2));
            }
            // set tex to point in descriptor vec
            color_tex = this->sampler_tex_map.at(color_tex);
        } else {
            // point to default color tex
            color_tex = 0;
        }

        // normal tex
        int normal_tex = primitive_data.tex_indexes.normal;
        if (normal_tex >= 0) {
            if (this->sampler_tex_map.count(normal_tex) == 0) {
                // tex does not yet exist as descriptor, create it
                this->sampler_tex_map.emplace(normal_tex, static_cast<int>(this->sampler_buffers.size()));
                this->sampler_buffers.push_back({});
                this->sampler_buffers.back().image_view().initialise(vkdata, this->model->vk_image_data()[normal_tex]);
                this->sampler_buffers.back().sampler().initialise(vkdata);
                this->sampler_buffers.back().initialise(vkdata, 0, this->pipeline->get_descriptor_set_layout(3));
            }
            // set tex to point in descriptor vec
            normal_tex = this->sampler_tex_map.at(normal_tex);
        } else {
            // @TODO: point to default normal tex
            normal_tex = 1;
        }

        // create m uniform buffer
        auto& ubo = this->m_uniform_buffers[index].emplace_back();
        ubo.data().transform = transform;
        ubo.initialise(vkdata, 0, this->pipeline->get_descriptor_set_layout(1));
        ubo.update_buffer(vkdata);

        // record render commands
        vkCmdBindPipeline(cmd_buffer(),
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          this->pipeline->get_pipeline(vkdata));

        std::array<VkDescriptorSet, 4> descriptor_sets = {
            this->vp_uniform_buffers[index].get_descriptor_set(index),
            ubo.get_descriptor_set(index),
            this->sampler_buffers[color_tex].get_descriptor_set(index),
            this->sampler_buffers[normal_tex].get_descriptor_set(index)
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
