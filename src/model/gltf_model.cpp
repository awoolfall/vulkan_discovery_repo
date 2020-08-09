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

    bool tex_coords = (atribs.count("TEXCOORD_0") != 0);
    bool colors = (atribs.count("COLOR_0") != 0);
    bool normals = (atribs.count("NORMAL") != 0);
    bool tangents = (atribs.count("TANGENT") != 0);

    std::vector<vertex> output;
    size_t vertex_count = model.accessors[atribs.at("POSITION")].count;
    output.resize(vertex_count);

    const float *pos_data, *color_data, *tex_data, *norm_data, *tangent_data;

    auto get_data = [model, atribs](std::string attrib, const float* &data_ptr){
        auto& accessor = model.accessors[atribs.at(attrib)];
        auto& buffer_view = model.bufferViews[accessor.bufferView];
        auto& buffer = model.buffers[buffer_view.buffer];
        data_ptr = reinterpret_cast<const float*>(&buffer.data[accessor.byteOffset + buffer_view.byteOffset]);
    };

    get_data("POSITION", pos_data);

    if (tex_coords) {
        int base_color_tex_index = model.materials[prim.material].pbrMetallicRoughness.baseColorTexture.texCoord;
        get_data(("TEXCOORD_" + std::to_string(base_color_tex_index)), tex_data);

        /* tex indicies */
        p.tex_indexes.color = model.materials[prim.material].pbrMetallicRoughness.baseColorTexture.index;
        p.tex_indexes.emissive = model.materials[prim.material].emissiveTexture.index;
        p.tex_indexes.metal_roughness = model.materials[prim.material].pbrMetallicRoughness.metallicRoughnessTexture.index;
        p.tex_indexes.normal = model.materials[prim.material].normalTexture.index;
    }

    if (colors) get_data("COLOR_0", color_data);
    if (normals) get_data("NORMAL", norm_data);
    if (tangents) get_data("TANGENT", tangent_data);

    for (size_t i = 0; i < vertex_count; i++) {
        output[i].position[0] =  pos_data[(i*3) + 0];
        output[i].position[1] =  -pos_data[(i*3) + 1];
        output[i].position[2] =  pos_data[(i*3) + 2];

        if (colors) {
            output[i].color[0] = color_data[(i*3) + 0];
            output[i].color[1] = color_data[(i*3) + 1];
            output[i].color[2] = color_data[(i*3) + 2];
        }

        if (tex_coords) {
            output[i].texcoord[0] = tex_data[(i*2) + 0];
            output[i].texcoord[1] = tex_data[(i*2) + 1];
        }

        if (normals) {
            output[i].normal[0] = norm_data[(i*3) + 0];
            output[i].normal[1] = -norm_data[(i*3) + 1];
            output[i].normal[2] = norm_data[(i*3) + 2];
        }

        if (tangents) {
            output[i].tangent[0] = tangent_data[(i*4) + 0];
            output[i].tangent[1] = -tangent_data[(i*4) + 1];
            output[i].tangent[2] = tangent_data[(i*4) + 2];
            output[i].tangent[3] = tangent_data[(i*4) + 3];
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
    auto& pos_accessor = model.accessors[atribs.at("POSITION")];
    p.prim_bounds.max.x = static_cast<float>(pos_accessor.maxValues[0]);
    p.prim_bounds.max.y = static_cast<float>(pos_accessor.maxValues[1]);
    p.prim_bounds.max.z = static_cast<float>(pos_accessor.maxValues[2]);

    p.prim_bounds.min.x = static_cast<float>(pos_accessor.minValues[0]);
    p.prim_bounds.min.y = static_cast<float>(pos_accessor.minValues[1]);
    p.prim_bounds.min.z = static_cast<float>(pos_accessor.minValues[2]);

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
    ret.max.x = ret.max.y = ret.max.z = -99999999999999.0f;
    ret.min.x = ret.min.y = ret.min.z =  99999999999999.0f;

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


std::vector<VkVertexInputAttributeDescription> vertex::get_attribute_descriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    VkVertexInputAttributeDescription desc;
    
    desc.binding = 0;
    desc.location = 0;
    desc.format = VK_FORMAT_R32G32B32_SFLOAT;
    desc.offset = offsetof(vertex, position);
    attributeDescriptions.push_back(desc);

    desc.binding = 0;
    desc.location = 1;
    desc.format = VK_FORMAT_R32G32B32_SFLOAT;
    desc.offset = offsetof(vertex, color);
    attributeDescriptions.push_back(desc);

    desc.binding = 0;
    desc.location = 2;
    desc.format = VK_FORMAT_R32G32_SFLOAT;
    desc.offset = offsetof(vertex, texcoord);
    attributeDescriptions.push_back(desc);

    desc.binding = 0;
    desc.location = 3;
    desc.format = VK_FORMAT_R32G32B32_SFLOAT;
    desc.offset = offsetof(vertex, normal);
    attributeDescriptions.push_back(desc);

    desc.binding = 0;
    desc.location = 4;
    desc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    desc.offset = offsetof(vertex, tangent);
    attributeDescriptions.push_back(desc);

    return attributeDescriptions;
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