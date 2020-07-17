#ifndef DISCOVERY_MODEL_3D_H
#define DISCOVERY_MODEL_3D_H

#include "gltf_reader.h"
#include "glm/glm.hpp"
#include "vulkan/vulkan_base.h"
#include "basic_pipeline.h"

struct model_3d {
    bool is_loaded = false;
    std::vector<mesh_3d> meshes;
    std::vector<node_3d> nodes;
    gltf_reader reader;

    void initialise_gltf(vulkan_data& data, const std::string& rel_path);
    void load_model(vulkan_data& data);
    void unload_model(vulkan_data& data);
    void terminate(vulkan_data& data);
    void set_view(vulkan_data& data, const glm::mat4& view);
    void set_proj(vulkan_data& data, const glm::mat4& proj);
};


#endif //DISCOVERY_MODEL_3D_H
