#include "mesh.h"
#include "platform.h"
#include <iostream>
#include "vulkan/vulkan.h"

#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION // defined in vulkan_image.cpp
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <include/tiny_gltf.h>

void mesh::initialise(const std::string &relative_path) {
    this->relative_path = relative_path;
    this->relative_path.erase(this->relative_path.find_last_of("/\\")+1);
    this->relative_path += "/";
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

    const float *pos_data, *tex_data;

    auto& pos_accessor = this->model.accessors[atribs.at("POSITION")];
    auto& pos_buffer_view = this->model.bufferViews[pos_accessor.bufferView];
    auto& pos_buffer = this->model.buffers[pos_buffer_view.buffer];
    pos_data = reinterpret_cast<const float*>(&pos_buffer.data[pos_accessor.byteOffset + pos_buffer_view.byteOffset]);

    if (tex_coords) {
        int base_color_tex_index = this->model.materials[this->model.meshes[mesh_index].primitives[0].material].pbrMetallicRoughness.baseColorTexture.texCoord;
        auto &tex_a = this->model.accessors[atribs.at(("TEXCOORD_" + std::to_string(base_color_tex_index)))];
        auto &tex_bv = this->model.bufferViews[tex_a.bufferView];
        auto &tex_b = this->model.buffers[tex_bv.buffer];
        tex_data = reinterpret_cast<const float*>(&tex_b.data[tex_a.byteOffset + tex_bv.byteOffset]);
    }

    for (size_t i = 0; i < vertex_count; i++) {
        size_t i_b = i * 3;
        output[i].position[0] =  pos_data[i_b + 0];
        output[i].position[1] = -pos_data[i_b + 1];
        output[i].position[2] =  pos_data[i_b + 2];

        if (tex_coords) {
            size_t i_t = i * 2;
            output[i].texcoord_color[0] = tex_data[i_t + 0];
            output[i].texcoord_color[1] = tex_data[i_t + 1];
        }
    }

    return output;
}

std::vector<uint32_t> mesh::create_indicies_array(uint32_t mesh_index) const {
    auto& accessor = this->model.accessors[this->model.meshes[mesh_index].primitives[0].indices];
    auto& buffer_view = this->model.bufferViews[accessor.bufferView];
    auto& buffer = this->model.buffers[buffer_view.buffer];
    std::cout << accessor.type << " " << accessor.componentType << std::endl;
    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
        const auto* i_data = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + buffer_view.byteOffset]);
        std::vector<uint32_t> output(i_data, i_data + accessor.count);
        return output;
    } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
        const auto* i_data = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + buffer_view.byteOffset]);
        std::vector<uint32_t> output(i_data, i_data + accessor.count);
        return output;
    }
}

vulkan_image mesh::create_color_tex(vulkan_data& vkdata, uint32_t mesh_index) const {
    std::cout << this->model.meshes[mesh_index].primitives[0].material << std::endl;
    auto& mat = this->model.materials[this->model.meshes[mesh_index].primitives[0].material];
    if (mat.pbrMetallicRoughness.baseColorTexture.index < 0) {
        throw std::runtime_error("Uses default texture, todo"); // @TODO: add functionality to default mat
    }
    auto& color_tex = this->model.textures[mat.pbrMetallicRoughness.baseColorTexture.index];
    auto& image = this->model.images[color_tex.source];
    std::cout << image.uri << " " << image.bufferView << " " << image.width << " " << image.height << std::endl;
    if (image.bufferView < 0) {
        // it is a file
        vulkan_image vkimage{};
        vkimage.initialise(vkdata, to_absolute_path(this->relative_path + image.uri));
        return vkimage;
    } else {
        // buffer of data
        vulkan_image vkimage{};
        vkimage.initialise(vkdata, image.image.data(), image.image.size());
        return vkimage;
    }
}
