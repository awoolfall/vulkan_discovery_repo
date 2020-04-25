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

    vulkan_image image{};
    image.initialise(vkdata, to_absolute_path("res/images/texture.jpg"));

    basic_pipeline basic_p;
    basic_p.initialise(vkdata, vkdata.render_pass);

    auto& basic_mvp = basic_p.mvp_uniform_buffer;
    basic_mvp.data().model = glm::mat4(1.0);
    basic_mvp.data().view = glm::translate(glm::mat4(1.0), {0.0, 0.0, -5.0});
    basic_mvp.data().proj = glm::perspective(
            glm::radians(45.0f),
            vkdata.swap_chain_data.extent.width / (float) vkdata.swap_chain_data.extent.height,
            0.1f, 10.0f);
    basic_mvp.update_buffer(vkdata);

    auto& basic_sampler = basic_p.sampler_buffer;
    basic_sampler.sampler().initialise(vkdata);
    basic_sampler.image_view().initialise(vkdata, image);
    // @TODO: gotta repopulate the descriptor sets when we change an image_view. Should look into a way to do this automatically
    basic_p.populate_descriptor_sets(vkdata);

    std::vector<basic_pipeline::vertex> vert_data = {
            {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
    };

    std::vector<uint32_t> index_data = {0, 1, 2, 0, 2, 3};;

    dynamic_buffer<basic_pipeline::vertex> buffer;
    buffer.initialise(vkdata, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vert_data.size());
    buffer.fill_buffer(vkdata, vert_data);

    static_buffer<uint32_t> i_buf;
    i_buf.initialise(vkdata, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, index_data);

    triangle_cmd cmd;
    cmd.pipeline = &basic_p;
    cmd.vert_buffer = &buffer;
    cmd.index_buffer = &i_buf;
    cmd.initialise(vkdata);

    glm::vec3 cameraPos = {0.0, 0.0, 0.0};
    glm::vec3 cameraRot = {0.0, 0.0, 0.0};

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // update projection
        basic_mvp.data().proj = glm::perspective(
                glm::radians(45.0f),
                vkdata.swap_chain_data.extent.width / (float) vkdata.swap_chain_data.extent.height,
                0.1f, 10.0f);

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
            if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                if (glfwRawMouseMotionSupported())
                    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            } else {
                double cursorX, cursorY;
                glfwGetCursorPos(window, &cursorX, &cursorY);
                cameraRot.y += (float)glm::radians(-cursorY/2.0);
                cameraRot.x += (float)glm::radians(cursorX/2.0);

                basic_mvp.data().view = glm::translate(glm::mat4(1.0), {0.0, 0.0, -5.0});
                basic_mvp.data().view = glm::rotate(basic_mvp.data().view, cameraRot.y, {1, 0, 0});
                basic_mvp.data().view = glm::rotate(basic_mvp.data().view, cameraRot.x, {0, 1, 0});
                basic_mvp.data().view = glm::translate(basic_mvp.data().view, cameraPos);
            }
            glfwSetCursorPos(window, 0.0, 0.0);
        } else {
            if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_NORMAL)
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        buffer.fill_buffer(vkdata, vert_data);
        basic_mvp.update_buffer(vkdata);

        submit_command_buffers_graphics(vkdata, cmd.cmd_buffers());
        present_frame(vkdata);
    }

    image.terminate(vkdata);
    buffer.terminate(vkdata);
    i_buf.terminate(vkdata);
    cmd.terminate(vkdata);
    basic_p.terminate(vkdata);
    terminate_vulkan(vkdata);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
