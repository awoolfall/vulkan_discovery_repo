#pragma once

#include <vector>
#include <array>
#include <string>
#include <stdexcept>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "vk_mem_alloc.h"
#include "dense_id_list.hpp"


class graphics_command_buffer;
class graphics_pipeline;
class uniform_buffer_base;
struct vulkan_image;
struct vulkan_image_view;

struct vkimage_and_allocation
{
    VkImage image;
    VmaAllocation allocation;
};


/* vulkan data */

#define MAX_FRAMES_IN_FLIGHT 2
struct vulkan_data
{
public:
    GLFWwindow* glfw_window;
    VkInstance instance = nullptr;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice logical_device = VK_NULL_HANDLE;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkSwapchainKHR swap_chain;
    VkRenderPass render_pass;
    VkCommandPool command_pool_graphics;
    struct {
        std::vector<VkImage> images;
        std::vector<VkImageView> image_views;
        std::vector<VkFramebuffer> frame_buffers;
        VkFormat image_format;
        VkExtent2D extent;
    } swap_chain_data;
    struct {
        VkImage image;
        VmaAllocation image_allocation;
        VkImageView image_view;
    } depth_resources;
    struct {
        VkImage image;
        VmaAllocation image_allocation;
        VkImageView image_view;
    } color_resources;
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> image_available_sems;
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> render_finished_sems;
    std::array<VkFence, MAX_FRAMES_IN_FLIGHT> in_flight_fences;
    int32_t image_index = -1;
    uint32_t current_frame = 0;
    std::vector<graphics_command_buffer*> registered_command_buffers;
    std::vector<graphics_pipeline*> registered_pipelines;
    VmaAllocator mem_allocator;
    vulkan_image* default_image;
    VkSampleCountFlagBits msaa_samples = VK_SAMPLE_COUNT_8_BIT;
};


enum class shader_type {
    VERTEX, FRAGMENT, GEOMETRY, COMPUTE
};


void initialise_vulkan(vulkan_data* data, GLFWwindow* window);
void terminate_vulkan(vulkan_data& data);

VkShaderModule create_shader_module_from_spirv(vulkan_data& vulkan, std::vector<char>& shader_data);
VkPipelineShaderStageCreateInfo gen_shader_stage_create_info(VkShaderModule module, shader_type type, const char* entry_point = "main");

void submit_command_buffers_graphics(vulkan_data& data, std::vector<VkCommandBuffer> command_buffers);
void present_frame(vulkan_data& data);

void register_command_buffer(vulkan_data& data, graphics_command_buffer* buffer);
void unregister_command_buffer(vulkan_data& data, graphics_command_buffer* buffer);
void register_pipeline(vulkan_data& data, graphics_pipeline* pipeline);
void unregister_pipeline(vulkan_data& data, graphics_pipeline* pipeline);

void create_buffer(vulkan_data& data, VkBuffer* buffer, VmaAllocation* allocation, VkDeviceSize byte_data_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage);
void fill_buffer(vulkan_data& vkdata, VmaAllocation& alloc, size_t data_length, void* data_start);
uint32_t get_image_index(vulkan_data& data);

VkPhysicalDeviceFeatures get_device_features(vulkan_data& data);

template <typename T>
void fill_buffer(vulkan_data& data, VmaAllocation& alloc, std::vector<T>& buffer_data)
{
    size_t data_size = buffer_data.size() * sizeof(buffer_data[0]);
    fill_buffer(data, alloc, data_size, buffer_data.data());
}


/* vulkan buffer */

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


/* vulkan command buffer */
class graphics_command_buffer
{
private:
    std::vector<VkCommandBuffer> command_buffers;
    uint32_t current_index = 0;
    uint32_t flags = 0x00 | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VkCommandBufferLevel buffer_level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

public:
    void initialise(vulkan_data& data);
    void reinitialise(vulkan_data& data);
    void reterminate(vulkan_data& data);
    void terminate(vulkan_data& data);
    std::vector<VkCommandBuffer>& cmd_buffers() {return command_buffers;}

protected:
    virtual void fill_command_buffer(vulkan_data& data, size_t index) = 0;
    virtual VkCommandBufferLevel get_buffer_level() const;
    virtual uint32_t get_flags() const;
    virtual void virtual_terminate(vulkan_data& vkdata) {};

    inline VkCommandBuffer& cmd_buffer() {
        return command_buffers[current_index];
    }

    inline VkFramebuffer& frame_buffer(vulkan_data& data) {
        return data.swap_chain_data.frame_buffers[current_index];
    }
    
};


/* vulkan pipeline */
struct uniform_buffer_decl {
    uint32_t set = 0;
    uint32_t binding = 0;
    VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    VkShaderStageFlagBits shaderFlags = VK_SHADER_STAGE_VERTEX_BIT;
};

class graphics_pipeline
{
private:
    VkRenderPass render_pass;
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    void clear_shader_stages(vulkan_data& data);

    std::vector<uniform_buffer_decl> uniformBufferDecls;
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
    std::vector<uniform_buffer_base*> allocated_uniform_buffers;

    void initialise_routine(vulkan_data& vkdata, VkRenderPass input_render_pass);
    void terminate_routine(vulkan_data& vkdata);

protected:
    VkPipeline pipeline;
    VkPipelineLayout layout;


    static VkDescriptorSetLayoutBinding create_descriptor_set_binding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags stages);

    virtual void gen_vertex_input_info(
            vulkan_data& data,
            std::vector<VkVertexInputBindingDescription>* binding_descriptions,
            std::vector<VkVertexInputAttributeDescription>* attrib_descriptions) = 0;
    virtual VkPipelineInputAssemblyStateCreateInfo gen_input_assembly_info(vulkan_data& data);
    virtual std::vector<VkViewport> gen_viewport(vulkan_data& data);
    virtual std::vector<VkRect2D> gen_scissor(vulkan_data& data);
    virtual VkPipelineRasterizationStateCreateInfo gen_rasterization_state_info(vulkan_data& data);
    virtual VkPipelineMultisampleStateCreateInfo gen_multisampling_state_info(vulkan_data& data);
    virtual VkPipelineColorBlendStateCreateInfo gen_color_blend_state(vulkan_data& data, std::vector<VkPipelineColorBlendAttachmentState>& attachment_states);
    virtual std::vector<VkDynamicState> gen_dynamic_state_info(vulkan_data& data);
    virtual std::vector<VkPipelineShaderStageCreateInfo> load_shader_stage_infos(vulkan_data& data);
    virtual std::vector<uniform_buffer_decl> get_uniform_buffer_declarations();

public:
    void initialise(vulkan_data& vkdata, VkRenderPass input_render_pass);
    void terminate(vulkan_data& data);
    void reinitialise(vulkan_data& vkdata, VkRenderPass input_render_pass);
    void reterminate(vulkan_data& vkdata);

    VkPipeline& get_pipeline(vulkan_data& vkdata);
    VkPipelineLayout& get_pipeline_layout();
    VkDescriptorSetLayout get_descriptor_set_layout(size_t set);
};


VkPipelineShaderStageCreateInfo gen_shader_stage_info_from_spirv(vulkan_data& data, std::string abs_path, shader_type type, const char* entry_point = "main");


template <typename T>
VkVertexInputBindingDescription get_binding_description(size_t binding)
{
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = (uint32_t)binding;
    bindingDescription.stride = sizeof(T);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}


template <typename T>
VkVertexInputBindingDescription get_binding_description_instanced(size_t binding)
{
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = (uint32_t)binding;
    bindingDescription.stride = sizeof(T);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
    return bindingDescription;
}


inline uniform_buffer_decl new_uniform_buffer_decl(uint32_t set, uint32_t binding, VkDescriptorType type, VkShaderStageFlagBits shaderFlags) {
    uniform_buffer_decl decl;
    decl.set = set;
    decl.binding = binding;
    decl.type = type;
    decl.shaderFlags = shaderFlags;
    return decl;
}


#define VERTEX_INPUT_DESCRIPTIONS(type) \
    static VkVertexInputBindingDescription get_binding_description(size_t binding){ return ::get_binding_description<type>(binding); } \
    static VkVertexInputBindingDescription get_binding_description_instanced(size_t binding){ return ::get_binding_description_instanced<type>(binding); } \
    static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();


/* vulkan image */
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

    void initialise(vulkan_data& vkdata, const VkImage& image);
    void terminate(vulkan_data& vkdata);
};


struct vulkan_sampler
{
    VkSampler sampler;

    void initialise(vulkan_data& vkdata);
    void terminate(vulkan_data& vkdata);
};


/* vulkan descriptor set */
struct uniform_info
{
    VkDescriptorType type;
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

    VkDescriptorPool descriptor_pool;
    std::vector<VkDescriptorSet> descriptor_sets;

protected:
    uint32_t uniform_binding = 0;

    virtual void virtual_initialise(vulkan_data& vk_data, uint32_t binding) {};
    virtual void virtual_terminate(vulkan_data& vk_data) {};
    virtual uniform_info get_uniform_info(size_t index) = 0;

    void update_descriptor_sets(vulkan_data& vkdata);

public:
    void initialise(vulkan_data &vkdata, uint32_t binding, VkDescriptorSetLayout descriptor_set_layout);
    void terminate(vulkan_data& vk_data);

    void rebuild_descriptor_sets(vulkan_data& vkdata, VkDescriptorSetLayout descriptor_set_layout);
    VkDescriptorSet get_descriptor_set(size_t index) const;

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
        info.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
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
    }

    void virtual_terminate(vulkan_data& data) final {
        this->image_view_data.terminate(data);
        this->sampler_data.terminate(data);
    }

    uniform_info get_uniform_info(size_t index) final {
        uniform_info info{};
        info.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
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

    void update_buffer(vulkan_data& vkdata) {
        this->update_descriptor_sets(vkdata);
    }
};
