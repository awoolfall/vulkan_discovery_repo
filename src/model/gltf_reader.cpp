#include "gltf_reader.h"
#include "platform.h"
#include <iostream>
#include "vulkan/vulkan.h"

#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION // defined in vulkan_image.cpp
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <include/tiny_gltf.h>
#include <glm/gtc/type_ptr.hpp>

struct gltf_reader::Impl {
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
};

gltf_reader::gltf_reader() {
    this->impl = new Impl;
}
gltf_reader::~gltf_reader() {
    delete this->impl;
}


void gltf_reader::initialise(const std::string &path) {
    this->relative_path = path;
    this->relative_path.erase(this->relative_path.find_last_of("/\\")+1);
    this->relative_path += "/";

    impl->loader.LoadASCIIFromFile(&impl->model, &this->err, &this->warn, to_absolute_path(path));
}

bool gltf_reader::is_valid() const {
    return this->err.empty();
}

uint32_t gltf_reader::get_position_data(const unsigned char* &data, uint32_t mesh_index, uint32_t primitive_index) const {
    if (impl->model.meshes[mesh_index].primitives[primitive_index].mode != 4) {
        throw std::runtime_error("Non triangle rendering mode is currently not supported");
    }
    auto ap = impl->model.meshes[mesh_index].primitives[primitive_index].attributes.at("POSITION");
    auto& accessor = impl->model.accessors[ap];
    auto& buffer_view = impl->model.bufferViews[ accessor.bufferView ];
    data = &impl->model.buffers[buffer_view.buffer].data[buffer_view.byteOffset + accessor.byteOffset];
    return buffer_view.byteLength;
}

uint32_t gltf_reader::get_num_meshes() const {
    return impl->model.meshes.size();
}

uint32_t gltf_reader::get_num_primitives(uint32_t mesh_index) const {
    return impl->model.meshes[mesh_index].primitives.size();
}

std::vector<vertex> gltf_reader::create_vertex_array(uint32_t mesh_index) const {
    if (impl->model.meshes[mesh_index].primitives[0].mode != 4) {
        throw std::runtime_error("Non triangle rendering mode is currently not supported");
    }
    auto& atribs = impl->model.meshes[mesh_index].primitives[0].attributes;

    bool tex_coords = false, colors = false;
    if (atribs.count("TEXCOORD_0") != 0) {
        tex_coords = true;
    }
    if (atribs.count("COLOR_0") != 0) {
        colors = true;
    }

    std::vector<vertex> output;
    size_t vertex_count = impl->model.accessors[atribs.at("POSITION")].count;
    output.resize(vertex_count);

    const float *pos_data, *tex_data;

    auto& pos_accessor = impl->model.accessors[atribs.at("POSITION")];
    auto& pos_buffer_view = impl->model.bufferViews[pos_accessor.bufferView];
    auto& pos_buffer = impl->model.buffers[pos_buffer_view.buffer];
    pos_data = reinterpret_cast<const float*>(&pos_buffer.data[pos_accessor.byteOffset + pos_buffer_view.byteOffset]);
    auto rot = impl->model.nodes[0].rotation;
    if (tex_coords) {
        int base_color_tex_index = impl->model.materials[impl->model.meshes[mesh_index].primitives[0].material].pbrMetallicRoughness.baseColorTexture.texCoord;
        auto &tex_a = impl->model.accessors[atribs.at(("TEXCOORD_" + std::to_string(base_color_tex_index)))];
        auto &tex_bv = impl->model.bufferViews[tex_a.bufferView];
        auto &tex_b = impl->model.buffers[tex_bv.buffer];
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

std::vector<uint32_t> gltf_reader::create_indicies_array(uint32_t mesh_index) const {
    auto& accessor = impl->model.accessors[impl->model.meshes[mesh_index].primitives[0].indices];
    auto& buffer_view = impl->model.bufferViews[accessor.bufferView];
    auto& buffer = impl->model.buffers[buffer_view.buffer];

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

vulkan_image gltf_reader::create_color_tex(vulkan_data& vkdata, uint32_t mesh_index) const {
    //std::cout << impl->model.meshes[mesh_index].primitives[0].material << std::endl;
    auto& mat = impl->model.materials[impl->model.meshes[mesh_index].primitives[0].material];
    if (mat.pbrMetallicRoughness.baseColorTexture.index < 0) {
        throw std::runtime_error("Uses default texture, todo"); // @TODO: add functionality to default mat
    }
    auto& color_tex = impl->model.textures[mat.pbrMetallicRoughness.baseColorTexture.index];
    auto& image = impl->model.images[color_tex.source];
    //std::cout << image.uri << " " << image.bufferView << " " << image.width << " " << image.height << std::endl;

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

std::vector<node_3d> gltf_reader::load_nodes() const {
    std::vector<node_3d> nodes;
    nodes.reserve(impl->model.nodes.size());
    for (auto& node : impl->model.nodes) {
        node_3d n;
        n.name = node.name;
        n.mesh_index = node.mesh;
        n.children_nodes = node.children;

        if (!node.matrix.empty()) {
            // do matrix
            n.transform = glm::make_mat4(node.matrix.data());
        } else {
            // do individual
            n.transform = glm::mat4(1.0);
            if (!node.scale.empty()) {
                n.transform = glm::scale(n.transform, {(float)node.scale[0], (float)node.scale[1], (float)node.scale[2]});
            }
            if (!node.rotation.empty()) {
                n.transform = n.transform * glm::mat4_cast(glm::quat((float)node.rotation[0], (float)node.rotation[1], (float)node.rotation[2], (float)node.rotation[3]));
            }
            if (!node.translation.empty()) {
                n.transform = glm::translate(n.transform, {(float)node.translation[0], (float)node.translation[1], (float)node.translation[2]});
            }
        }
        nodes.push_back(n);
    }
    return nodes;
}
