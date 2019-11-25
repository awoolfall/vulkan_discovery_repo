
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

struct velocity
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

glm::mat4 get_model_matrix(position& pos)
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3((float)pos.x, (float)pos.y, (float)pos.z));
    // mModelMatrix = glm::scale(mModelMatrix, glm::vec3(this->mScale));
    // mModelMatrix = glm::rotate(mModelMatrix, glm::radians(this->mRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    // mModelMatrix = glm::rotate(mModelMatrix, glm::radians(this->mRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    // mModelMatrix = glm::rotate(mModelMatrix, glm::radians(this->mRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
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

    glViewport(0, 0, width, height);
    glClearColor(0.85f, 0.9f, 0.9f, 0.0f);

    entt::registry Registry;

    auto e = Registry.create();
    Registry.assign<position>(e);
    // Registry.view<position, velocity> ().each([](position& pos, velocity& vel){
    //     pos.x += vel.x;
    //     pos.y += vel.y;
    // });

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

    glm::mat4 proj = glm::ortho((-(float)width/2.0f)/100.0f, ((float)width/2.0f)/100.0f, (-(float)height/2.0f)/100.0f, ((float)height/2.0f)/100.0f, 0.01f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -1.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));

    unsigned int model_location = glGetUniformLocation(shader, "model");
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        Registry.view<position> ().each([model_location](position& pos){
            glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(get_model_matrix(pos)));
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