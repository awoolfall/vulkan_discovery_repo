
#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <entt/entity/registry.hpp>

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
    int x = 0, y = 0;
};

struct velocity
{
    double x = 1.0, y = 2.0;
};

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
    Registry.assign<velocity>(e);
    Registry.view<position, velocity> ().each([](position& pos, velocity& vel){
        pos.x += (int)vel.x;
        pos.y += (int)vel.y;
        printf("%d, %d\n", pos.x, pos.y);
    });

    GLuint vbo, vao;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), &quad_verticies, GL_STATIC_DRAW);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLuint shader = glCreateProgram();
    attach_combined_glsl(shader, to_absolute_path("res/shaders/basic.glsl").c_str());

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 18*sizeof(float));

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