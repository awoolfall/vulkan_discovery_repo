
#pragma once

#include "vulkan_base.h"

struct vulkan_image
{
    VkImage image;
    VmaAllocation allocation;
    static const VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;

    void initialise(vulkan_data& vkdata, const unsigned char* data, size_t data_length);
    void initialise(vulkan_data& vkdata, std::string abs_file_path);
    void initialise_default(vulkan_data& data);
    void terminate(vulkan_data& vkdata);

private:
    void initialise_with_staging_buffer(vulkan_data& data, VkBuffer* stagingBuffer, VmaAllocation* stagingAllocation, uint32_t texWidth, uint32_t texHeight);
};

struct vulkan_image_view
{
    VkImageView imageView;

    void initialise(vulkan_data& vkdata, vulkan_image& image);
    void terminate(vulkan_data& vkdata);
};

struct vulkan_sampler
{
    VkSampler sampler;

    void initialise(vulkan_data& vkdata);
    void terminate(vulkan_data& vkdata);
};
