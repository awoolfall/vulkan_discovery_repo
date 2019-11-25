
#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <entt/entity/registry.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "platform.h"

#include <stdio.h>

const float quad_verticies[18] = {
    -0.5f, 0.5f, 0.0f,
    0.5f, 0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
    0.5f, 0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f
};

struct position
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

struct transform
{
    struct { float x=0.0f, y=0.0f, z=0.0f; } position;
    struct { float x=0.0f, y=0.0f, z=0.0f; } rotation;
    struct { float x=1.0f, y=1.0f, z=1.0f; } scale;
};

struct velocity
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

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

int main(int argc, char* argv)
{
    constexpr unsigned int width = 1600, height = 900;
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(width, height, "Discovery", NULL, NULL);
    if (window == nullptr) {
        printf("Failed to create GLFW window, closing\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    /* the following allows opengl textures to not have to be 4 byte aligned (helps with font loading) */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, width, height);
    glClearColor(0.85f, 0.9f, 0.9f, 0.0f);

    GLuint vbo, vao;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), &quad_verticies, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));

    GLuint shader = glCreateProgram();
    attach_combined_glsl(shader, to_absolute_path("res/shaders/basic_shader.glsl").c_str());
    glUseProgram(shader);

    glm::mat4 proj = glm::ortho((-(float)width/2.0f)/100.0f, ((float)width/2.0f)/100.0f, (-(float)height/2.0f)/100.0f, ((float)height/2.0f)/100.0f, -100.0f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));

    entt::registry Registry;
    auto [e, t] = Registry.create<transform>();
    t.position.x = -2.0f;
    Registry.create<transform>();

    glm::vec3 cam_pos(0.0f, 0.0f, 0.0f);
    unsigned int model_location = glGetUniformLocation(shader, "model");
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);

        if (glfwGetKey(window, GLFW_KEY_W)) {
            cam_pos.y -= 0.001f;
        }
        if (glfwGetKey(window, GLFW_KEY_S)) {
            cam_pos.y += 0.001f;
        }
        if (glfwGetKey(window, GLFW_KEY_A)) {
            cam_pos.x += 0.001f;
        }
        if (glfwGetKey(window, GLFW_KEY_D)) {
            cam_pos.x -= 0.001f;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
            cam_pos.z += 0.001f;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
            cam_pos.z -= 0.001f;
        }
        view = glm::mat4(1.0f);
        view = glm::translate(view, cam_pos);
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));

        Registry.view<transform> ().each([model_location](transform& t){
            glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(get_model_matrix(t)));
            glDrawArrays(GL_TRIANGLES, 0, 6);
        });

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shader);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}