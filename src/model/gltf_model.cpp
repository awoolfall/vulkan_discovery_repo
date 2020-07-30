#include "gltf_model.h"

#include <platform.h>

#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION // defined in vulkan_image.cpp
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <include/tiny_gltf.h>
#include <glm/gtc/type_ptr.hpp>


void gltf_model::initialise(const std::string& path)
{
    this->relative_path = path;
    this->relative_path.erase(this->relative_path.find_last_of("/\\")+1);

    this->_is_valid = this->loader.LoadASCIIFromFile(&this->gltf_model, &this->err, &this->warn, ::to_absolute_path(path));
}

void gltf_model::terminate(vulkan_data& vkdata)
{
    if (this->is_loaded())
        this->unload_model(vkdata);
}

prim_data load_prim(vulkan_data& vkdata, tinygltf::Model& model, tinygltf::Primitive& prim)
{
    if (prim.mode != 4) {
        throw std::runtime_error("Non triangle rendering mode is currently not supported");
    }

    prim_data p;

    /* set vertex buffer */
    auto& atribs = prim.attributes;

    bool tex_coords = false, colors = false;
    if (atribs.count("TEXCOORD_0") != 0) {
        tex_coords = true;
    }
    if (atribs.count("COLOR_0") != 0) {
        colors = true;
    }

    std::vector<vertex> output;
    size_t vertex_count = model.accessors[atribs.at("POSITION")].count;
    output.resize(vertex_count);

    const float *pos_data, *tex_data;

    auto& pos_accessor = model.accessors[atribs.at("POSITION")];
    auto& pos_buffer_view = model.bufferViews[pos_accessor.bufferView];
    auto& pos_buffer = model.buffers[pos_buffer_view.buffer];
    pos_data = reinterpret_cast<const float*>(&pos_buffer.data[pos_accessor.byteOffset + pos_buffer_view.byteOffset]);
    if (tex_coords) {
        p.color_tex = model.materials[prim.material].pbrMetallicRoughness.baseColorTexture.index;
        int base_color_tex_index = model.materials[prim.material].pbrMetallicRoughness.baseColorTexture.texCoord;
        auto &tex_a = model.accessors[atribs.at(("TEXCOORD_" + std::to_string(base_color_tex_index)))];
        auto &tex_bv = model.bufferViews[tex_a.bufferView];
        auto &tex_b = model.buffers[tex_bv.buffer];
        tex_data = reinterpret_cast<const float*>(&tex_b.data[tex_a.byteOffset + tex_bv.byteOffset]);
    }

    for (size_t i = 0; i < vertex_count; i++) {
        size_t i_b = i * 3;
        output[i].position[0] =  pos_data[i_b + 0];
        output[i].position[1] =  pos_data[i_b + 1];
        output[i].position[2] =  pos_data[i_b + 2];

        if (tex_coords) {
            size_t i_t = i * 2;
            output[i].texcoord_color[0] = tex_data[i_t + 0];
            output[i].texcoord_color[1] = tex_data[i_t + 1];
        }
    }

    p.vertex_buffer.initialise(vkdata, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, output);

    /* index buffer */
    if (prim.indices >= 0) {
        p.has_index_buffer = true;

        auto& accessor = model.accessors[prim.indices];
        auto& buffer_view = model.bufferViews[accessor.bufferView];
        auto& buffer = model.buffers[buffer_view.buffer];

        std::vector<uint32_t> output;
        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
            const auto* i_data = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + buffer_view.byteOffset]);
            output = std::vector<uint32_t>(i_data, i_data + accessor.count);
        } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
            const auto* i_data = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + buffer_view.byteOffset]);
            output = std::vector<uint32_t>(i_data, i_data + accessor.count);
        }

        //p.index_buffer.initialise_static(vkdata, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, output.data(), output.size() * sizeof(uint32_t));
        p.index_buffer.initialise(vkdata, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, output);
    } else {
        p.has_index_buffer = false;
    }

    /* bounds */
    p.prim_bounds.max.x = pos_accessor.maxValues[0];
    p.prim_bounds.max.y = pos_accessor.maxValues[1];
    p.prim_bounds.max.z = pos_accessor.maxValues[2];

    p.prim_bounds.min.x = pos_accessor.minValues[0];
    p.prim_bounds.min.y = pos_accessor.minValues[1];
    p.prim_bounds.min.z = pos_accessor.minValues[2];

    /* return populated prim data */
    return p;
}

vulkan_image load_image(vulkan_data& vkdata, const std::string& gltf_path, tinygltf::Image& image)
{
    vulkan_image vkimage{};
    if (image.bufferView < 0) {
        // it is a file
        vkimage.initialise(vkdata, to_absolute_path(gltf_path + image.uri));
    } else {
        // buffer of data
        vkimage.initialise(vkdata, image.image.data(), image.image.size());
    }
    return vkimage;
}

void gltf_model::load_model(vulkan_data& vkdata)
{
    /* throw if model is already loaded */
    if (this->_is_loaded) {
        throw std::runtime_error("attempted to load model which has already been loaded");
    }

    /* load images */
    this->_image_data.reserve(this->gltf_model.images.size());
    for (size_t i = 0; i < this->gltf_model.images.size(); i++) {
        // load image
        auto vkim = load_image(vkdata, this->relative_path, this->gltf_model.images[i]);
        this->_image_data.push_back(vkim);
    }

    /* load meshes */
    this->_mesh_data.reserve(this->gltf_model.meshes.size());
    for (size_t m = 0; m < this->gltf_model.meshes.size(); m++) {
        // load mesh
        mesh_data md;
        md.primitive_data.reserve(this->gltf_model.meshes[m].primitives.size());
        for (size_t p = 0; p < this->gltf_model.meshes[m].primitives.size(); p++) {
            // load primative
            auto pd = load_prim(vkdata, this->gltf_model, this->gltf_model.meshes[m].primitives[p]);
            md.primitive_data.push_back(pd);
        }
        this->_mesh_data.push_back(md);
    }

    // @TODO: load materials

    this->_is_loaded = true;
}

void gltf_model::unload_model(vulkan_data& vkdata)
{
    /* return early if this model is not loaded */
    if (!this->_is_loaded) {
        throw std::runtime_error("attempted to unload model which has not yet loaded");
    }

    /* unload all images */
    for (vulkan_image& image : this->_image_data) {
        image.terminate(vkdata);
    }
    this->_image_data.clear();

    /* unload all meshes */
    for (mesh_data& mesh : this->_mesh_data) {
        for (prim_data& prim : mesh.primitive_data) {
            prim.vertex_buffer.terminate(vkdata);
            if (prim.has_index_buffer) {
                prim.index_buffer.terminate(vkdata);
            }
        }
    }
    this->_mesh_data.clear();

    this->_is_loaded = false;
}

bool gltf_model::is_loaded() const
{
    return this->_is_loaded;
}

const tinygltf::Model& gltf_model::model() const {
    return this->gltf_model;
}

const std::vector<mesh_data>& gltf_model::vk_mesh_data() const
{
    return this->_mesh_data;
}
    
const std::vector<vulkan_image>& gltf_model::vk_image_data() const
{
    return this->_image_data;
}

bounds gltf_model::get_model_bounds() const
{
    bounds ret;
    ret.max.x = ret.max.y = ret.max.z = -99999999999999999999.0;
    ret.min.x = ret.min.y = ret.min.z = 99999999999999999999.0;

    for (auto& m : this->_mesh_data) {
        for (auto& p : m.primitive_data) {
            ret.max.x = std::max(ret.max.x, p.prim_bounds.max.x);
            ret.max.y = std::max(ret.max.y, p.prim_bounds.max.y);
            ret.max.z = std::max(ret.max.z, p.prim_bounds.max.z);

            ret.min.x = std::min(ret.min.x, p.prim_bounds.min.x);
            ret.min.y = std::min(ret.min.y, p.prim_bounds.min.y);
            ret.min.z = std::min(ret.min.z, p.prim_bounds.min.z);
        }
    }
    return ret;
}



// void flatten(std::vector<node_3d>& nodes, node_3d* node, glm::mat4 parent_transform) {
//     node->transform = parent_transform * node->transform;
//     for (int child : node->children_nodes)
//         flatten(nodes, &nodes[child], node->transform);
// }

// std::vector<node_3d> gltf_reader::load_nodes() const {
//     std::vector<node_3d> nodes;
//     nodes.reserve(impl->model.nodes.size());
//     for (auto& node : impl->model.nodes) {
//         node_3d n;
//         n.name = node.name;
//         n.mesh_index = node.mesh;
//         n.children_nodes = node.children;

//         // do individual transforms
//         n.transform = glm::mat4(1.0);
//         if (!node.scale.empty()) {
//             n.transform = glm::scale(n.transform, {(float)node.scale[0], (float)node.scale[1], (float)node.scale[2]});
//         }
//         if (!node.rotation.empty()) {
//             n.transform = n.transform * glm::mat4_cast(glm::quat((float)node.rotation[0], (float)node.rotation[1], (float)node.rotation[2], (float)node.rotation[3]));
//         }
//         if (!node.translation.empty()) {
//             n.transform = glm::translate(n.transform, {(float)node.translation[0], (float)node.translation[1], (float)node.translation[2]});
//         }

//         nodes.push_back(n);
//     }
//     flatten(nodes, &nodes[0], glm::mat4(1.0));
//     return nodes;
// }