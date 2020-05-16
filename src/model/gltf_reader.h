#ifndef DISCOVERY_GLTF_READER_H
#define DISCOVERY_GLTF_READER_H

#include <glm/glm.hpp>
#include <basic_pipeline.h>
#include "vulkan/vulkan_image.h"
#include "vulkan/vulkan_buffer.h"
#include "vulkan/vulkan_graphics_pipeline.h"

struct vertex {
    glm::vec3 position = glm::vec3(0.0);
    glm::vec3 color = glm::vec3(0.0);
    glm::vec2 texcoord_color = glm::vec2(0.0);

    VERTEX_INPUT_DESCRIPTIONS(vertex);
};

struct mesh_3d {
    static_buffer<vertex> vertex_buffer;
    static_buffer<uint32_t> index_buffer;
    vulkan_image color_texture;
    // @TODO: add pbr rendering and textures, do some model type (pbr, color, etc.) identifying i guess
};

struct node_3d {
    std::string name = "";
    int mesh_index = -1;
    glm::mat4 transform = glm::mat4(1.0);
    std::vector<int> children_nodes{};
    basic_pipeline pipeline;

    inline bool has_mesh() const {return mesh_index >= 0;}
};

class gltf_reader {
private:
    struct Impl; Impl* impl;

public:
    std::string relative_path = "";
    std::string err;
    std::string warn;

    gltf_reader();
    ~gltf_reader();

    void initialise(const std::string& relative_path);
    bool is_valid() const;

    uint32_t get_position_data(const unsigned char* &data, uint32_t mesh_index, uint32_t primitive_index) const;
    uint32_t get_num_meshes() const;
    uint32_t get_num_primitives(uint32_t mesh_index) const;

    std::vector<node_3d> load_nodes() const;

    std::vector<vertex> create_vertex_array(uint32_t mesh_index) const;
    std::vector<uint32_t> create_indicies_array(uint32_t mesh_index) const;
    vulkan_image create_color_tex(vulkan_data& data, uint32_t mesh_index) const;
};


#endif //DISCOVERY_GLTF_READER_H
