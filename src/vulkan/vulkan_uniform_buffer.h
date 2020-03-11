#pragma once
#include "vulkan_base.h"

class uniform_buffer_base
{
public:
    virtual void recreate_descriptor_sets(vulkan_data& data, VkDescriptorSetLayout& descriptor_set_layout) = 0;
    virtual std::vector<VkDescriptorSet>& get_descriptor_sets() = 0;
};

template <typename T>
class uniform_buffer : public uniform_buffer_base
{
private:
    T uniform_data{};
    uint32_t uniform_binding{};
    VkDescriptorSetLayout set_layout{};

    std::vector<VkBuffer> uniform_buffers;
    std::vector<VmaAllocation> allocations;
    VkDescriptorPool descriptor_pool{};
    std::vector<VkDescriptorSet> descriptor_sets;

    void create_descriptor_pool(vulkan_data& vkdata, uint32_t descriptor_count, uint32_t num_sets)
    {
        VkDescriptorPoolSize poolSize = {};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = descriptor_count;

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = num_sets;

        if (vkCreateDescriptorPool(vkdata.logical_device, &poolInfo, nullptr, &descriptor_pool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void create_descriptor_sets(vulkan_data& vkdata, uint32_t num_swap_chain_images)
    {
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

    void populate_descriptor_sets(vulkan_data& vkdata)
    {
        for (size_t i = 0; i < vkdata.swap_chain_data.images.size(); i++) {
            VkDescriptorBufferInfo bufferInfo = {};
            bufferInfo.buffer = this->uniform_buffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(T);

            VkWriteDescriptorSet descriptorWrite = {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptor_sets[i];
            descriptorWrite.dstBinding = this->uniform_binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
            descriptorWrite.pImageInfo = nullptr; // Optional
            descriptorWrite.pTexelBufferView = nullptr; // Optional

            vkUpdateDescriptorSets(vkdata.logical_device, 1, &descriptorWrite, 0, nullptr);
        }
    }

public:
    inline T& data() {
        return this->uniform_data;
    }

    void initialise(vulkan_data& vk_data, uint32_t binding, VkDescriptorSetLayout descriptor_set_layout)
    {
        this->uniform_binding = binding;
        this->set_layout = descriptor_set_layout;

        VkDeviceSize buffer_size = sizeof(T);
        size_t num_swap_chain_images = vk_data.swap_chain_data.images.size();
        uniform_buffers.resize(num_swap_chain_images);
        allocations.resize(num_swap_chain_images);

        for (size_t i = 0; i < num_swap_chain_images; i++) {
            ::create_buffer(vk_data, &uniform_buffers[i], &allocations[i], buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        }

        create_descriptor_pool(vk_data, num_swap_chain_images, num_swap_chain_images);
        create_descriptor_sets(vk_data, num_swap_chain_images);
        populate_descriptor_sets(vk_data);
    }

    void terminate(vulkan_data& vk_data)
    {
        vkDeviceWaitIdle(vk_data.logical_device);
        for (size_t i = 0; i < uniform_buffers.size(); i++) {
            vmaDestroyBuffer(vk_data.mem_allocator, uniform_buffers[i], allocations[i]);
        }
        vkDestroyDescriptorPool(vk_data.logical_device, this->descriptor_pool, nullptr);
    }

    void update_buffer(vulkan_data& vk_data)
    {
        uint32_t image_index = ::get_image_index(vk_data);

        void* map_data;
        vmaMapMemory(vk_data.mem_allocator, allocations[image_index], &map_data);
        memcpy(map_data, &this->uniform_data, sizeof(T));
        vmaUnmapMemory(vk_data.mem_allocator, allocations[image_index]);
    }

    void recreate_descriptor_sets(vulkan_data& vk_data, VkDescriptorSetLayout& descriptor_set_layout) final
    {
        if (!this->descriptor_sets.empty()) {
            vkDestroyDescriptorPool(vk_data.logical_device, this->descriptor_pool, nullptr);
            this->descriptor_sets.clear();
        }
        uint32_t num_swap_chain_images = static_cast<uint32_t> (vk_data.swap_chain_data.images.size());
        create_descriptor_pool(vk_data, num_swap_chain_images, num_swap_chain_images);
        create_descriptor_sets(vk_data, num_swap_chain_images);
    }

    std::vector<VkDescriptorSet>& get_descriptor_sets() final
    {
        return descriptor_sets;
    }
};