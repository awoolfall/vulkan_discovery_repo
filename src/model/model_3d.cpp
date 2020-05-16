#include "model_3d.h"

void model_3d::initialise_gltf(vulkan_data& vkdata, const std::string& rel_path) {
    this->gltf.initialise(rel_path);

    // initialise model nodes
    this->nodes = this->gltf.load_nodes();
}

void model_3d::terminate(vulkan_data &data) {
    if (is_loaded) {
        this->unload_model(data);
    }
}

void model_3d::load_model(vulkan_data &data) {
    if (is_loaded) {
        throw std::runtime_error("Attempted to load model which is already loaded");
    }

    auto num = this->gltf.get_num_meshes();
    this->meshes.resize(num);

    for (size_t i = 0; i < num; i++) {
        auto vertex_array = this->gltf.create_vertex_array(i);
        this->meshes[i].vertex_buffer.initialise(data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertex_array);
        auto index_array = this->gltf.create_indicies_array(i);
        this->meshes[i].index_buffer.initialise(data, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, index_array);
        this->meshes[i].color_texture = this->gltf.create_color_tex(data, i);
    }

    for (auto& node : this->nodes) {
        if (node.has_mesh()) {
            node.pipeline.initialise(data, data.render_pass);
            node.pipeline.mvp_uniform_buffer.data().model = node.transform;
            node.pipeline.mvp_uniform_buffer.update_buffer(data);

            node.pipeline.sampler_buffer.sampler().initialise(data);
            node.pipeline.sampler_buffer.image_view().initialise(data, this->meshes[node.mesh_index].color_texture);
            node.pipeline.sampler_buffer.update_buffer(data);
        }
    }

    is_loaded = true;
}

void model_3d::unload_model(vulkan_data &data) {
    if (!is_loaded) {
        throw std::runtime_error("Attempted to unload model which is not loaded");
    }

    // terminate node pipelines
    for (auto& node : this->nodes) {
        if (node.has_mesh()) {
            node.pipeline.terminate(data);
        }
    }

    for (auto &mesh : this->meshes) {
        mesh.vertex_buffer.terminate(data);
        mesh.index_buffer.terminate(data);
        mesh.color_texture.terminate(data);
    }
    this->meshes.clear();
    this->meshes.shrink_to_fit();
    is_loaded = false;
}

void model_3d::set_view(vulkan_data& data, const glm::mat4 &view) {
    for (auto& node : this->nodes) {
        if (node.has_mesh()) {
            node.pipeline.mvp_uniform_buffer.data().view = view;
            node.pipeline.mvp_uniform_buffer.update_buffer(data);
        }
    }
}

void model_3d::set_proj(vulkan_data &data, const glm::mat4 &proj) {
    for (auto& node : this->nodes) {
        if (node.has_mesh()) {
            node.pipeline.mvp_uniform_buffer.data().proj = proj;
            node.pipeline.mvp_uniform_buffer.update_buffer(data);
        }
    }
}
