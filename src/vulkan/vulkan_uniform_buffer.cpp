#include "vulkan_uniform_buffer.h"

void
uniform_buffer_base::initialise(vulkan_data &vk_data, uint32_t binding, VkDescriptorSetLayout descriptor_set_layout) {
    this->uniform_binding = binding;
    this->set_layout = descriptor_set_layout;

    this->virtual_initialise(vk_data, binding, descriptor_set_layout);

    size_t num_swap_chain_images = vk_data.swap_chain_data.images.size();

    create_descriptor_pool(vk_data, num_swap_chain_images, num_swap_chain_images);
    create_descriptor_sets(vk_data, num_swap_chain_images);
    populate_descriptor_sets(vk_data);

    register_uniform_buffer(vk_data, this);
}

void uniform_buffer_base::terminate(vulkan_data &vk_data) {
    vkDeviceWaitIdle(vk_data.logical_device);
    unregister_uniform_buffer(vk_data, this);

    this->virtual_terminate(vk_data);

    vkDestroyDescriptorPool(vk_data.logical_device, this->descriptor_pool, nullptr);
}

void uniform_buffer_base::create_descriptor_pool(vulkan_data &vkdata, uint32_t descriptor_count, uint32_t num_sets) {
    auto poolSize = this->gen_descriptor_pool_size(vkdata);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = num_sets;

    if (vkCreateDescriptorPool(vkdata.logical_device, &poolInfo, nullptr, &descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void uniform_buffer_base::create_descriptor_sets(vulkan_data &vkdata, uint32_t num_swap_chain_images) {
    std::vector<VkDescriptorSetLayout> layouts(num_swap_chain_images, this->set_layout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = this->descriptor_pool;
    allocInfo.descriptorSetCount = num_swap_chain_images;
    allocInfo.pSetLayouts = layouts.data();

    descriptor_sets.resize(num_swap_chain_images);
    if (vkAllocateDescriptorSets(vkdata.logical_device, &allocInfo, this->descriptor_sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
}

void uniform_buffer_base::recreate_descriptor_sets(vulkan_data &data, VkDescriptorSetLayout &descriptor_set_layout) {
    if (!this->descriptor_sets.empty()) {
        vkDestroyDescriptorPool(data.logical_device, this->descriptor_pool, nullptr);
        this->descriptor_sets.clear();
    }
    auto num_swap_chain_images = static_cast<uint32_t> (data.swap_chain_data.images.size());
    create_descriptor_pool(data, num_swap_chain_images, num_swap_chain_images);
    create_descriptor_sets(data, num_swap_chain_images);
}

std::vector<VkDescriptorSet> &uniform_buffer_base::get_descriptor_sets() {
    return this->descriptor_sets;
}
