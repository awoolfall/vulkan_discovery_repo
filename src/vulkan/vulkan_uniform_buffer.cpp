#include "vulkan_uniform_buffer.h"

void
uniform_buffer_base::initialise(vulkan_data &vkdata, uint32_t binding) {
    this->uniform_binding = binding;
    this->virtual_initialise(vkdata, binding);
    this->shouldRepopulateDescSets = true;
}

void uniform_buffer_base::terminate(vulkan_data &vk_data) {
    vkDeviceWaitIdle(vk_data.logical_device);
    this->virtual_terminate(vk_data);
}

bool uniform_buffer_base::should_repopulate_descriptor_sets() {
    if (this->shouldRepopulateDescSets) {
        this->shouldRepopulateDescSets = false;
        return true;
    }
    return false;
}
