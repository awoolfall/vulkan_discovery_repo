#pragma once
#include <glm/glm.hpp>
#include "transform_methods.inl"

struct transform
{
    struct { float x=0.0f, y=0.0f, z=0.0f; } position;
    struct { float x=0.0f, y=0.0f, z=0.0f; } rotation;
    struct { float x=1.0f, y=1.0f, z=1.0f; } scale;
};

glm::vec3 get_up_vector(transform &t);
glm::vec3 get_forward_vector(transform &t);
glm::vec3 get_right_vector(transform &t);

glm::mat4 get_model_matrix(transform& t);
glm::mat4 get_view_matrix(transform &t);