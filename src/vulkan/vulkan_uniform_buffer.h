#pragma once
#include "vulkan_base.h"
#include "vulkan_image.h"

struct uniform_info
{
    enum Type {
        UNIFORM_BUFFER,
        SAMPLER_BUFFER
    } type;
    uint32_t binding = 0;
    union {
        VkDescriptorBufferInfo bufferInfo;
        VkDescriptorImageInfo imageInfo;
    };
};

class uniform_buffer_base
{
private:
    friend class graphics_pipeline;

    bool should_repopulate_descriptor_sets();

protected:
    uint32_t uniform_binding = 0;
    bool shouldRepopulateDescSets = true;

    virtual void virtual_initialise(vulkan_data& vk_data, uint32_t binding) {};
    virtual void virtual_terminate(vulkan_data& vk_data) {};
    virtual uniform_info get_uniform_info(size_t index) = 0;

public:
    void initialise(vulkan_data &vkdata, uint32_t binding);
    void terminate(vulkan_data& vk_data);

};

template <typename T>
class uniform_buffer : public uniform_buffer_base
{
private:
    T uniform_data{};

    std::vector<VkBuffer> uniform_buffers;
    std::vector<VmaAllocation> allocations;

protected:
    void virtual_initialise(vulkan_data& vk_data, uint32_t binding) final {
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

    uniform_info get_uniform_info(size_t index) final {
        uniform_info info{};
        info.type = uniform_info::UNIFORM_BUFFER;
        info.binding = this->uniform_binding;

        info.bufferInfo.buffer = this->uniform_buffers[index];
        info.bufferInfo.offset = 0;
        info.bufferInfo.range = sizeof(T);
        return info;
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
    void virtual_initialise(vulkan_data& vk_data, uint32_t binding) final {
        image_view_data.initialise(vk_data, *vk_data.default_image);
        sampler_data.initialise(vk_data);
    }

    void virtual_terminate(vulkan_data& data) final {
        image_view_data.terminate(data);
        sampler_data.terminate(data);
    }

    uniform_info get_uniform_info(size_t index) final {
        uniform_info info{};
        info.type = uniform_info::SAMPLER_BUFFER;
        info.binding = this->uniform_binding;

        info.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        info.imageInfo.imageView = this->image_view_data.imageView;
        info.imageInfo.sampler = this->sampler_data.sampler;
        return info;
    }

public:
    inline vulkan_image_view& image_view() {
        return this->image_view_data;
    }

    inline vulkan_sampler& sampler() {
        return this->sampler_data;
    }

    void update_buffer(vulkan_data& vk_data) {
        this->shouldRepopulateDescSets = true;
    }
};
