#pragma once
#include "vulkan_base.h"
#include "vulkan_image.h"

class uniform_buffer_base
{
private:
    void create_descriptor_pool(vulkan_data& vkdata, uint32_t descriptor_count, uint32_t num_sets);
    void create_descriptor_sets(vulkan_data& vkdata, uint32_t num_swap_chain_images);

protected:
    uint32_t uniform_binding{};
    VkDescriptorSetLayout set_layout{};

    VkDescriptorPool descriptor_pool{};
    std::vector<VkDescriptorSet> descriptor_sets;

    virtual void virtual_initialise(vulkan_data& vk_data, uint32_t binding, VkDescriptorSetLayout descriptor_set_layout) {};
    virtual void virtual_terminate(vulkan_data& vk_data) {};
    virtual VkDescriptorPoolSize gen_descriptor_pool_size(vulkan_data& data) const = 0;
    virtual void populate_descriptor_sets(vulkan_data &vkdata) = 0;

public:
    void initialise(vulkan_data& vk_data, uint32_t binding, VkDescriptorSetLayout descriptor_set_layout);
    void terminate(vulkan_data& vk_data);
    void recreate_descriptor_sets(vulkan_data& data, VkDescriptorSetLayout& descriptor_set_layout);
    std::vector<VkDescriptorSet>& get_descriptor_sets();

};

template <typename T>
class uniform_buffer : public uniform_buffer_base
{
private:
    T uniform_data{};

    std::vector<VkBuffer> uniform_buffers;
    std::vector<VmaAllocation> allocations;

protected:
    void virtual_initialise(vulkan_data& vk_data, uint32_t binding, VkDescriptorSetLayout descriptor_set_layout) final {
        VkDeviceSize buffer_size = sizeof(T);
        size_t num_swap_chain_images = vk_data.swap_chain_data.images.size();
        uniform_buffers.resize(num_swap_chain_images);
        allocations.resize(num_swap_chain_images);

        for (size_t i = 0; i < num_swap_chain_images; i++) {
            ::create_buffer(vk_data, &uniform_buffers[i], &allocations[i], buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        }
    }

    void virtual_terminate(vulkan_data& vk_data) final {
        for (size_t i = 0; i < uniform_buffers.size(); i++) {
            vmaDestroyBuffer(vk_data.mem_allocator, uniform_buffers[i], allocations[i]);
        }
    }

    VkDescriptorPoolSize gen_descriptor_pool_size(vulkan_data& data) const final {
        VkDescriptorPoolSize poolSize = {};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(data.swap_chain_data.images.size());
        return poolSize;
    }

    void populate_descriptor_sets(vulkan_data &vkdata) final {
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

    void update_buffer(vulkan_data& vk_data) {
        uint32_t image_index = ::get_image_index(vk_data);

        void* map_data;
        vmaMapMemory(vk_data.mem_allocator, allocations[image_index], &map_data);
        memcpy(map_data, &this->uniform_data, sizeof(T));
        vmaUnmapMemory(vk_data.mem_allocator, allocations[image_index]);
    }
};

class sampler_uniform_buffer : public uniform_buffer_base
{
private:
    vulkan_image_view image_view_data{};
    vulkan_sampler sampler_data{};

protected:
    VkDescriptorPoolSize gen_descriptor_pool_size(vulkan_data& data) const final {
        VkDescriptorPoolSize poolSize = {};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = static_cast<uint32_t>(data.swap_chain_data.images.size());
        return poolSize;
    };

    void populate_descriptor_sets(vulkan_data &vkdata) final {
        for (size_t i = 0; i < vkdata.swap_chain_data.images.size(); i++) {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = this->image_view_data.imageView;
            imageInfo.sampler = this->sampler_data.sampler;

            VkWriteDescriptorSet descriptorWrite = {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptor_sets[i];
            descriptorWrite.dstBinding = this->uniform_binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = nullptr;
            descriptorWrite.pImageInfo = &imageInfo; // Optional
            descriptorWrite.pTexelBufferView = nullptr; // Optional

            vkUpdateDescriptorSets(vkdata.logical_device, 1, &descriptorWrite, 0, nullptr);
        }
    };

public:
    inline vulkan_image_view& image_view() {
        return this->image_view_data;
    }

    inline vulkan_sampler& sampler() {
        return this->sampler_data;
    }

    void update_buffer(vulkan_data& vk_data) {
        this->populate_descriptor_sets(vk_data);
    }
};
