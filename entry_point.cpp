#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>

#include <entt/entity/registry.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "src/vulkan/vulkan_base.h"
#include "src/basic_pipeline.h"
#include "src/basic_command_buffer.h"
#include "src/platform.h"
#include "src/transform.h"
#include "model/gltf_model.h"

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

template <typename T>
T lerpValue(T start, T end, double t) {
    return (start + (t * (end - start)));
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
    
//        std::vector<vertex> qvert_data = {
//                {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//                {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//                {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
//                {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
//
//                {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//                {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//                {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
//                {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
//        };
    
    // std::vector<uint32_t> qindex_data = {0, 1, 2, 2, 3, 0,
    //     4, 5, 6, 6, 7, 4};

    basic_pipeline pipeline;
    pipeline.initialise(vkdata, vkdata.render_pass);

    gltf_model gmodel;
    gmodel.initialise("res/models/pony/scene.gltf");
    //gmodel.initialise("res/models/car.gltf");
    gmodel.load_model(vkdata);

    triangle_cmd cmd;
    cmd.model = &gmodel;
    cmd.pipeline = &pipeline;
    cmd.initialise(vkdata);
    
    auto gmodel_bounds = gmodel.get_model_bounds();

    glm::vec3 cameraPos = {0.0, 0.0, 0.0};
    glm::vec3 cameraRot = {0.0, 0.0, 0.0};
    float cameraZoom = -3000.0f;
    float desiredCameraZoom = -1.0f * std::max({gmodel_bounds.max.x, gmodel_bounds.max.y, gmodel_bounds.max.z});
    
    double timeLastFrame = glfwGetTime();
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        double deltaTime;
        {
            double t = glfwGetTime();
            deltaTime = t - timeLastFrame;
            timeLastFrame = t;
        }

        int h,w;
        glfwGetWindowSize(window, &w, &h);
        cmd.projection_matrix = glm::perspective(45.0, ((double)w / (double)h), 0.1, 10000.0);
        
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
            }
            glfwSetCursorPos(window, 0.0, 0.0);
        } else {
            if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_NORMAL)
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            desiredCameraZoom -= deltaTime * 100.0;
        } else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            desiredCameraZoom += deltaTime * 100.0;
        }

        cameraZoom = lerpValue(cameraZoom, desiredCameraZoom, deltaTime * 5.0);

        glm::mat4 v = glm::translate(glm::mat4(1.0), {0.0, 0.0, cameraZoom});
        v = glm::rotate(v, cameraRot.y, {1, 0, 0});
        v = glm::rotate(v, cameraRot.x, {0, 1, 0});
        v = glm::translate(v, cameraPos);
        cmd.view_matrix = v;
        // @TODO: set v to somewhere


        cmd.reterminate(vkdata);
        cmd.reinitialise(vkdata); // @TODO: make it only update what is necessary

        submit_command_buffers_graphics(vkdata, cmd.cmd_buffers());
        present_frame(vkdata);
    }

    gmodel.terminate(vkdata);
    cmd.terminate(vkdata);
    pipeline.terminate(vkdata);
    terminate_vulkan(vkdata);
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
