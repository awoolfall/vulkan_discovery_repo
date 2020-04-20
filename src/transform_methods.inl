
#ifndef DISCOVERY_TRANSFORM_METHODS_INL
#define DISCOVERY_TRANSFORM_METHODS_INL

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>

inline static
bool decompose_model(const glm::mat4& model, glm::vec3& scale, glm::quat& orientation, glm::vec3& translation) {
    glm::vec3 n3; glm::vec4 n4;
    bool res = glm::decompose(model, scale, orientation, translation, n3, n4);
    orientation = glm::conjugate(orientation);
    return res;
}

inline static
void local_translate_model(glm::mat4& mat, const glm::vec3& translation) {
    mat = glm::translate(mat, translation);
}

inline static
void local_rotate_model(glm::mat4& mat, const glm::vec3& rotation_rads) {
    auto translation = glm::vec3(mat[3]);
    mat[3][0] = mat[3][1] = mat[3][2] = 0.0;

    if (rotation_rads.x != 0.0)
        mat = glm::rotate(mat, rotation_rads.x, {1, 0, 0});
    if (rotation_rads.y != 0.0)
        mat = glm::rotate(mat, rotation_rads.y, {0, 1, 0});
    if (rotation_rads.z != 0.0)
        mat = glm::rotate(mat, rotation_rads.z, {0, 0, 1});

    mat[3][0] = translation[0];
    mat[3][1] = translation[1];
    mat[3][2] = translation[2];
}

inline static
bool expensive_scale_mat4(glm::mat4& mat, const glm::vec3& new_scale) {
    glm::vec3 n3;
    glm::quat orientation;
    glm::vec3 translation;
    if (decompose_model(mat, n3, orientation, translation)) {
        mat = glm::mat4();
        mat = glm::scale(mat, new_scale);
        mat = mat * glm::mat4_cast(orientation);
        mat = glm::translate(mat, translation);
        return true;
    }
    return false;
}

inline static
glm::vec3 get_local_up_vector(const glm::mat4& model)
{
    return model * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
}

inline static
glm::vec3 get_local_forward_vector(const glm::mat4& model)
{
    return model * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
}

inline static
glm::vec3 get_local_right_vector(const glm::mat4& model)
{
    return model * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
}

#endif //DISCOVERY_TRANSFORM_METHODS_INL
