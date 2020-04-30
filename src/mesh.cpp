#include "mesh.h"
#include "platform.h"
#include <iostream>
#include "vulkan/vulkan.h"

#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION // defined in vulkan_image.cpp
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <include/tiny_gltf.h>

void mesh::initialise(const std::string &relative_path) {
    this->loader.LoadASCIIFromFile(&this->model, &this->err, &this->warn, to_absolute_path(relative_path));
}

bool mesh::is_valid() const {
    return this->err.empty();
}

uint32_t mesh::get_position_data(const unsigned char* &data, uint32_t mesh_index, uint32_t primitive_index) const {
    if (this->model.meshes[mesh_index].primitives[primitive_index].mode != 4) {
        throw std::runtime_error("Non triangle rendering mode is currently not supported");
    }
    auto ap = this->model.meshes[mesh_index].primitives[primitive_index].attributes.at("POSITION");
    auto& accessor = this->model.accessors[ap];
    auto& buffer_view = this->model.bufferViews[ accessor.bufferView ];
    data = &this->model.buffers[buffer_view.buffer].data[buffer_view.byteOffset + accessor.byteOffset];
    return buffer_view.byteLength;
}

uint32_t mesh::get_num_meshes() const {
    return this->model.meshes.size();
}

uint32_t mesh::get_num_primitives(uint32_t mesh_index) const {
    return this->model.meshes[mesh_index].primitives.size();
}

std::vector<vertex> mesh::create_vertex_array(uint32_t mesh_index) const {
    if (this->model.meshes[mesh_index].primitives[0].mode != 4) {
        throw std::runtime_error("Non triangle rendering mode is currently not supported");
    }
    auto& atribs = this->model.meshes[mesh_index].primitives[0].attributes;

    bool tex_coords = false, colors = false;
    if (atribs.count("TEXCOORD_0") != 0) {
        tex_coords = true;
    }
    if (atribs.count("COLOR_0") != 0) {
        colors = true;
    }

    std::vector<vertex> output;
    size_t vertex_count = this->model.accessors[atribs.at("POSITION")].count;
    output.resize(vertex_count);

    auto& pos_accessor = this->model.accessors[atribs.at("POSITION")];
    auto& pos_buffer_view = this->model.bufferViews[pos_accessor.bufferView];
    auto& pos_buffer = this->model.buffers[pos_buffer_view.buffer];
    const auto* pos_data = reinterpret_cast<const float*>(&pos_buffer.data[pos_accessor.byteOffset + pos_buffer_view.byteOffset]);

    if (tex_coords) {
        auto &tex_a = this->model.accessors[atribs.at("TEXCOORD_0")];

    }
    for (size_t i = 0; i < vertex_count; i++) {
        size_t i_b = i * 3;
        output[i].position[0] = pos_data[i_b + 0];
        output[i].position[1] = pos_data[i_b + 1];
        output[i].position[2] = pos_data[i_b + 2];
    }

    return output;
}

std::vector<uint16_t> mesh::create_indicies_array(uint32_t mesh_index) const {
    auto& accessor = this->model.accessors[this->model.meshes[mesh_index].primitives[0].indices];
    auto& buffer_view = this->model.bufferViews[accessor.bufferView];
    auto& buffer = this->model.buffers[buffer_view.buffer];
    const auto* i_data = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + buffer_view.byteOffset]);
    std::vector<uint16_t> output(i_data, i_data + accessor.count);
    return output;
}
