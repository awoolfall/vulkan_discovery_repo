#ifndef DISCOVERY_MESH_H
#define DISCOVERY_MESH_H

#include <glm/glm.hpp>
#include <include/tiny_gltf.h>
#include "vulkan/vulkan_image.h"

struct vertex {
    glm::vec3 position = glm::vec3(0.0);
    glm::vec3 color = glm::vec3(0.0);
    glm::vec2 texcoord_color = glm::vec2(0.0);
};

struct mesh {
    std::string relative_path = "";
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err;
    std::string warn;

    void initialise(const std::string& relative_path);
    bool is_valid() const;

    uint32_t get_position_data(const unsigned char* &data, uint32_t mesh_index, uint32_t primitive_index) const;
    uint32_t get_num_meshes() const;
    uint32_t get_num_primitives(uint32_t mesh_index) const;

    std::vector<vertex> create_vertex_array(uint32_t mesh_index) const;
    std::vector<uint32_t> create_indicies_array(uint32_t mesh_index) const;
    vulkan_image create_color_tex(vulkan_data& data, uint32_t mesh_index) const;
};


#endif //DISCOVERY_MESH_H
