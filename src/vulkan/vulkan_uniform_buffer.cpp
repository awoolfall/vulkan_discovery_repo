#include "vulkan_base.h"

void uniform_buffer_base::initialise(vulkan_data &vkdata, uint32_t binding) {
    this->uniform_binding = binding;
    this->virtual_initialise(vkdata, binding);
    this->rebuild_descriptor_sets(vkdata, descriptor_set_layout);
}

void uniform_buffer_base::terminate(vulkan_data &vkdata) {
    vkDeviceWaitIdle(vkdata.logical_device);
    this->virtual_terminate(vkdata);
    vkDestroyDescriptorPool(vkdata.logical_device, this->descriptor_pool, nullptr);
}

void uniform_buffer_base::rebuild_descriptor_sets(vulkan_data& vkdata, VkDescriptorSetLayout descriptor_set_layout)
{
    // clear descriptor pool if it already exists
    if (!this->descriptor_sets.empty()) {
        vkDestroyDescriptorPool(vkdata.logical_device, this->descriptor_pool, nullptr);
    }

    auto num_swap_chain_images = static_cast<uint32_t>(vkdata.swap_chain_data.images.size());

    // generate descriptor pool sizes
    VkDescriptorPoolSize poolSize{};
    poolSize.type = this->get_uniform_info(0).type;
    poolSize.descriptorCount = num_swap_chain_images;

    // create descriptor pool
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = num_swap_chain_images;

    if (vkCreateDescriptorPool(vkdata.logical_device, &poolInfo, nullptr, &this->descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    // create descriptor sets
    std::vector<VkDescriptorSetLayout> layouts(num_swap_chain_images, descriptor_set_layout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = this->descriptor_pool;
    allocInfo.descriptorSetCount = num_swap_chain_images;
    allocInfo.pSetLayouts = layouts.data();

    this->descriptor_sets.resize(num_swap_chain_images);
    if (vkAllocateDescriptorSets(vkdata.logical_device, &allocInfo, this->descriptor_sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    this->update_descriptor_sets(vkdata);
}

void uniform_buffer_base::update_descriptor_sets(vulkan_data& vkdata) {
    // create writeDescriptorSets and update
    for (size_t i = 0; i < vkdata.swap_chain_data.images.size(); i++) {
        auto& info = this->get_uniform_info(i);

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = this->descriptor_sets[i];
        descriptorWrite.dstBinding = this->uniform_binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorCount = 1;

        descriptorWrite.descriptorType = info.type;
        descriptorWrite.pBufferInfo = &info.bufferInfo;
        descriptorWrite.pImageInfo = &info.imageInfo;

        vkUpdateDescriptorSets(vkdata.logical_device, 1, &descriptorWrite, 0, nullptr);
    }
}
