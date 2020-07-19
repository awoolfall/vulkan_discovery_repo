#pragma once

#include "vulkan_base.h"

class buffer_base
{
protected:
    bool is_static = false;
    VkBuffer buffer{};
    VmaAllocation allocation{};
    size_t max_byte_size = 0;

public:
    void initialise_static(vulkan_data& data, VkBufferUsageFlagBits usage, void* vertex_data, size_t byte_size);
    void initialise_dynamic(vulkan_data& data, VkBufferUsageFlagBits usage, size_t byte_size);
    void terminate(vulkan_data& data);

    bool fill_buffer(vulkan_data& vkdata, void* data, size_t byte_size);

    VkBuffer get_vk_buffer() const;
    size_t byte_size() const;
};

template <typename T>
class static_buffer : public buffer_base
{
protected:
    size_t count = 0;

public:
    void initialise(vulkan_data& data, VkBufferUsageFlagBits usage, std::vector<T> input_data)
    {
        this->count = input_data.size();
        this->initialise_static(data, usage, input_data.data(), input_data.size() * sizeof(T));
    }

    size_t get_count() const {
        return this->count; 
    }
};

template <typename T>
class dynamic_buffer : public buffer_base
{
protected:
    size_t count = 0;

public:
    void initialise(vulkan_data& data, VkBufferUsageFlagBits usage, size_t data_length)
    {
        this->count = data_length;
        this->initialise_dynamic(data, usage, data_length * sizeof(T));
    }

    bool fill_buffer(vulkan_data& vkdata, std::vector<T> input_data)
    {
        return this->fill_buffer(vkdata, input_data.data(), input_data.size() * sizeof(T));
    }

    size_t get_count() const {
        return this->count; 
    }
};
