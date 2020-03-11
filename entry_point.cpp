#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>

#include <entt/entity/registry.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "src/vulkan/vulkan_graphics.h"
#include "src/basic_pipeline.h"
#include "src/basic_command_buffer.h"
// #include "src/shader.h"
#include "src/platform.h"
#include "src/transform.h"

#include <stdio.h>
#include <iostream>

const float quad_verticies[18] = {
    -0.5f, 0.5f, 0.0f,
    0.5f, 0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
    0.5f, 0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f
};

void perform_camera_control(GLFWwindow* window, entt::registry& Registry, entt::entity Camera)
{
    transform& cam_transform = Registry.get<transform> (Camera);
    glm::vec3 movement_vector(0.0f);
    if (glfwGetKey(window, GLFW_KEY_W)) {
        movement_vector.y += 0.001f;
    }
    if (glfwGetKey(window, GLFW_KEY_S)) {
        movement_vector.y -= 0.001f;
    }
    if (glfwGetKey(window, GLFW_KEY_A)) {
        movement_vector.x -= 0.001f;
    }
    if (glfwGetKey(window, GLFW_KEY_D)) {
        movement_vector.x += 0.001f;
    }
    if (glfwGetKey(window, GLFW_KEY_E)) {
        cam_transform.rotation.z += 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_Q)) {
        cam_transform.rotation.z -= 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
        movement_vector.z += 0.001f;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
        movement_vector.z -= 0.001f;
    }
    glm::vec3 screen_space_movement = get_right_vector(cam_transform) * movement_vector.x;
    screen_space_movement += (get_up_vector(cam_transform) * movement_vector.y);
    screen_space_movement += (get_forward_vector(cam_transform) * movement_vector.z);
    cam_transform.position.x += screen_space_movement.x;
    cam_transform.position.y += screen_space_movement.y;
    cam_transform.position.z += screen_space_movement.z;
}

int main(int argc, char** argv)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

    vulkan_data vkdata;
    try{
        initialise_vulkan(&vkdata, window);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    basic_pipeline basic_p;
    basic_p.initialise(vkdata, vkdata.render_pass);

    auto basic_ubo = basic_p.new_mvp_ubo(vkdata);
    basic_ubo.data().model = glm::rotate(
            glm::mat4(1.0f),
            glm::radians(40.0f),
            glm::vec3(0.0f, 0.0f, 1.0f));
    basic_ubo.data().view = glm::lookAt(
            glm::vec3(2.0f, 2.0f, 2.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f));
    basic_ubo.data().proj =glm::perspective(
            glm::radians(45.0f),
            vkdata.swap_chain_data.extent.width / (float) vkdata.swap_chain_data.extent.height,
            0.1f, 10.0f);
    basic_ubo.data().proj[1][1] *= -1;
    basic_ubo.update_buffer(vkdata);

    std::vector<basic_pipeline::vertex> vert_data = {
        {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.9f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
    };

    dynamic_buffer<basic_pipeline::vertex> buffer;
    buffer.initialise(vkdata, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vert_data.size());
    buffer.fill_buffer(vkdata, vert_data);

    triangle_cmd cmd;
    cmd.pipeline = &basic_p;
    cmd.vert_buffer = &buffer;
    cmd.uniform_buffers = &basic_ubo;
    cmd.initialise(vkdata);

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        vert_data[2].pos.x = (float)sin(glfwGetTime());
        buffer.fill_buffer(vkdata, vert_data);
        basic_ubo.update_buffer(vkdata);

        submit_command_buffers_graphics(vkdata, cmd.cmd_buffers());
        present_frame(vkdata);
    }

    basic_ubo.terminate(vkdata);
    buffer.terminate(vkdata);
    cmd.terminate(vkdata);
    basic_p.terminate(vkdata);
    terminate_vulkan(vkdata);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
