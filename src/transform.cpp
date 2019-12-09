#include "transform.h"
#include <glm/gtc/matrix_transform.hpp>

glm::vec3 get_up_vector(transform &t)
{
    glm::mat4 model_matrix = get_model_matrix(t);
    return model_matrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
}

glm::vec3 get_forward_vector(transform &t)
{
    glm::mat4 model_matrix = get_model_matrix(t);
    return model_matrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
}

glm::vec3 get_right_vector(transform &t)
{
    glm::mat4 model_matrix = get_model_matrix(t);
    return model_matrix * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
}

glm::mat4 get_model_matrix(transform& t)
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(t.position.x, t.position.y, t.position.z));
    model = glm::scale(model, glm::vec3(t.scale.x, t.scale.y, t.scale.z));
    model = glm::rotate(model, glm::radians(t.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(t.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(t.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    return model;
}

glm::mat4 get_view_matrix(transform &t)
{
    glm::mat4 view(1.0f);
    view = glm::translate(view, glm::vec3(t.position.x, t.position.y, t.position.z));
    view = glm::rotate(view, glm::radians(t.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    view = glm::rotate(view, glm::radians(t.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::rotate(view, glm::radians(t.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    view = glm::inverse(view);
    return view;
}