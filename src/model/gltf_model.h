#pragma once

#include <glm/glm.hpp>
#include <basic_pipeline.h>
#include "vulkan/vulkan_base.h"
#include <include/tiny_gltf.h>

struct vertex {
    glm::vec3 position = glm::vec3(0.0);
    glm::vec3 color = glm::vec3(0.0);
    glm::vec2 texcoord_color = glm::vec2(0.0);

    VERTEX_INPUT_DESCRIPTIONS(vertex);
};

struct prim_data {
    static_buffer<vertex> vertex_buffer;
    static_buffer<uint32_t> index_buffer;
    bool has_index_buffer = false;
};

struct mesh_data {
    std::vector<prim_data> primitive_data;
};

class gltf_model {
private:
    std::string relative_path = "";
    tinygltf::TinyGLTF loader;
    tinygltf::Model gltf_model;
    bool _is_valid = false;
    bool _is_loaded = false;

    std::vector<mesh_data> _mesh_data;
    std::vector<vulkan_image> _image_data;

public:
    std::string err = "";
    std::string warn = "";

    void initialise(const std::string& relative_path);
    void terminate(vulkan_data& vkdata);

    void load_model(vulkan_data& vkdata);
    void unload_model(vulkan_data& vkdata);

    bool is_valid() const;
    bool is_loaded() const;

    const tinygltf::Model& model() const;
    const std::vector<mesh_data>& vk_mesh_data() const;
    const std::vector<vulkan_image>& vk_image_data() const;
};
