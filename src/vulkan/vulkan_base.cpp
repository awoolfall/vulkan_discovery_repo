#include "vulkan_base.h"
#include "vulkan_command_buffer.h"
#include "vulkan_graphics_pipeline.h"
#include "vulkan_uniform_buffer.h"

#include "../platform.h"

#include <fstream>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"


struct physical_device_indicies
{
    bool has_graphics_queue = false;
    uint32_t graphics_queue_index = 0;
    bool has_present_queue = false;
    uint32_t present_queue_index = 0;
};

struct swap_chain_support_details
{
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

void create_instance(vulkan_data* data)
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &appInfo;
    create_info.enabledLayerCount = 0;
    uint32_t num_extensions = 0;
    auto ext = glfwGetRequiredInstanceExtensions(&num_extensions);
    create_info.enabledExtensionCount = num_extensions;
    create_info.ppEnabledExtensionNames = ext;

#ifndef NDEBUG
    /* validation layers */
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_LUNARG_standard_validation"
    };
    create_info.enabledLayerCount = (uint32_t)validationLayers.size();
    create_info.ppEnabledLayerNames = validationLayers.data();

    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, layers.data());
    bool found_layer = false;
    for (const auto& layer : layers) {
        if (strcmp(layer.layerName, validationLayers[0]) == 0) {
            found_layer = true;
            break;
        }
    }
    if (!found_layer) {
        throw std::runtime_error("Requested layer does not exist");
    }
#endif

    if (vkCreateInstance(&create_info, nullptr, &data->instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create vulkan instance");
    }
}

void create_surface(vulkan_data* data, GLFWwindow* window)
{
    if (glfwCreateWindowSurface(data->instance, window, nullptr, &data->surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

physical_device_indicies get_device_indicies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    physical_device_indicies indicies{};

    uint32_t num_queue_families = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &num_queue_families, nullptr);
    std::vector<VkQueueFamilyProperties> queue_family_properties(num_queue_families);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &num_queue_families, queue_family_properties.data());
    int index = 0;
    for (VkQueueFamilyProperties prop : queue_family_properties) {
        if (prop.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indicies.has_graphics_queue = true;
            indicies.graphics_queue_index = index;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &present_support);
        if (present_support) {
            indicies.has_present_queue = true;
            indicies.present_queue_index = index;
        }

        index++;
    }

    return indicies;
}

bool check_device_extension_support(VkPhysicalDevice device, std::vector<const char*>& deviceExtensions)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::vector<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        for (size_t i = 0; i < requiredExtensions.size(); i++) {
            if (strcmp(requiredExtensions[i].c_str(), extension.extensionName) == 0) {
                requiredExtensions.erase(requiredExtensions.begin()+i);
                break;
            }
        }
    }
 
    return requiredExtensions.empty();
}

swap_chain_support_details query_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    swap_chain_support_details details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.present_modes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.present_modes.data());
    }

    return details;
}

bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<const char*>& required_extensions)
{
    auto indicies = get_device_indicies(device, surface);
    bool ext = check_device_extension_support(device, required_extensions);
    bool swap_chain_is_adequate = false;
    if (ext) {
        auto details = query_swap_chain_support(device, surface);
        swap_chain_is_adequate = !details.formats.empty() && !details.present_modes.empty();
    }
    return indicies.has_graphics_queue && ext && swap_chain_is_adequate;
}

void pick_physical_device(vulkan_data* data, std::vector<const char*>& required_extensions)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(data->instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("None of the available hardware supports vulkan!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(data->instance, &deviceCount, devices.data());

    for (VkPhysicalDevice device : devices) {
        if (is_device_suitable(device, data->surface, required_extensions)) {
            data->physical_device = device;
            break;
        }
    }

    if (data->physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("Unable to select a suitable physical device!");
    }
}

void create_logical_device(vulkan_data* data, std::vector<const char*>& required_extensions)
{
    auto queue_indicies = get_device_indicies(data->physical_device, data->surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::vector<uint32_t> uniqueQueueFamilies = {queue_indicies.graphics_queue_index, queue_indicies.present_queue_index};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = required_extensions.data();

    if (vkCreateDevice(data->physical_device, &deviceCreateInfo, nullptr, &data->logical_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(data->logical_device, queue_indicies.graphics_queue_index, 0, &data->graphics_queue);
    vkGetDeviceQueue(data->logical_device, queue_indicies.present_queue_index, 0, &data->present_queue);
}

VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats)
{
    for (auto& format : available_formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return available_formats[0]; // @TODO: add in additional stuff here in case this exact format does not exist. (i.e. it is probably best to ensure BGRA)
}

VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR>& available_modes)
{
    // choose mailbox, if that doesnt exist return fifo
    for (auto& mode : available_modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

constexpr inline uint32_t get_max(uint32_t a, uint32_t b){ return (a > b ? a : b); }
constexpr inline uint32_t get_min(uint32_t a, uint32_t b){ return (a < b ? a : b); }
VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height)
{
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actual_extent = {width, height};
        actual_extent.width = get_max(capabilities.minImageExtent.width, get_min(capabilities.maxImageExtent.width, actual_extent.width));
        actual_extent.height = get_max(capabilities.minImageExtent.height, get_min(capabilities.maxImageExtent.height, actual_extent.height));
        return actual_extent;
    }
}

void create_swap_chain(vulkan_data* data, uint32_t width, uint32_t height)
{
    auto swap_chain_support = query_swap_chain_support(data->physical_device, data->surface);
    auto surface_format = choose_swap_surface_format(swap_chain_support.formats);
    auto present_mode = choose_present_mode(swap_chain_support.present_modes);
    auto extent = choose_swap_extent(swap_chain_support.capabilities, width, height);

    uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
    if (swap_chain_support.capabilities.maxImageCount > 0) { // 0 means there is no maximum image count
        image_count = get_min(image_count, swap_chain_support.capabilities.maxImageCount);
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = data->surface;
    createInfo.minImageCount = image_count;
    createInfo.imageFormat = surface_format.format;
    createInfo.imageColorSpace = surface_format.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = swap_chain_support.capabilities.currentTransform;

    // specify ownership for sharing images across queue families
    auto indices = get_device_indicies(data->physical_device, data->surface);
    uint32_t queueFamilyIndices[] = {indices.graphics_queue_index, indices.present_queue_index};
    if (indices.graphics_queue_index != indices.present_queue_index) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    // window blending!
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = present_mode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    if (vkCreateSwapchainKHR(data->logical_device, &createInfo, nullptr, &data->swap_chain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    uint32_t swap_chain_image_count;
    vkGetSwapchainImagesKHR(data->logical_device, data->swap_chain, &swap_chain_image_count, nullptr);
    data->swap_chain_data.images.resize(swap_chain_image_count);
    vkGetSwapchainImagesKHR(data->logical_device, data->swap_chain, &swap_chain_image_count, data->swap_chain_data.images.data());

    data->swap_chain_data.extent = extent;
    data->swap_chain_data.image_format = surface_format.format;
}

void create_swap_chain_image_views(vulkan_data* data)
{
    data->swap_chain_data.image_views.resize(data->swap_chain_data.images.size());
    for (size_t i = 0; i < data->swap_chain_data.image_views.size(); i++) {
        VkImageViewCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = data->swap_chain_data.images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = data->swap_chain_data.image_format;

        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(data->logical_device, &create_info, nullptr, &data->swap_chain_data.image_views[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

void create_render_pass(vulkan_data* data)
{
    // @TODO this will have to be updated in the future for depth passes, post processing etc.
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = data->swap_chain_data.image_format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass = {};  
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(data->logical_device, &renderPassInfo, nullptr, &data->render_pass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void create_frame_buffers(vulkan_data* data)
{
    // @FIX framebuffers and render passes, this will need to be generalised at some point probably
    data->swap_chain_data.frame_buffers.resize(data->swap_chain_data.image_views.size());
    for (size_t i = 0; i < data->swap_chain_data.image_views.size(); i++) {
        VkImageView attachments[] = {
            data->swap_chain_data.image_views[i]
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = data->render_pass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = data->swap_chain_data.extent.width;
        framebufferInfo.height = data->swap_chain_data.extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(data->logical_device, &framebufferInfo, nullptr, &data->swap_chain_data.frame_buffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void create_command_pools(vulkan_data* data)
{
    auto indicies = get_device_indicies(data->physical_device, data->surface);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = indicies.graphics_queue_index;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

    if (vkCreateCommandPool(data->logical_device, &poolInfo, nullptr, &data->command_pool_graphics) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void initialise_memory_allocator(vulkan_data* data)
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = data->physical_device;
    allocatorInfo.device = data->logical_device;
    
    if (vmaCreateAllocator(&allocatorInfo, &data->mem_allocator) != VK_SUCCESS) {
        throw std::runtime_error("Unable to initialise VK memory allocator!");
    }
}

void terminate_memory_allocator(vulkan_data& data)
{
    vmaDestroyAllocator(data.mem_allocator);
}

void create_semaphores(vulkan_data* data, std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT>& semaphores)
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(data->logical_device, &semaphoreInfo, nullptr, &semaphores[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        }
    }
}

void create_fences(vulkan_data* data, std::array<VkFence, MAX_FRAMES_IN_FLIGHT>& fences)
{
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateFence(data->logical_device, &fenceInfo, nullptr, &fences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        }
    }
}

void cleanup_swap_chain(vulkan_data* data)
{
    for (auto framebuffer : data->swap_chain_data.frame_buffers) {
        vkDestroyFramebuffer(data->logical_device, framebuffer, nullptr);
    }
    vkDestroyRenderPass(data->logical_device, data->render_pass, nullptr);
    for (auto image_view : data->swap_chain_data.image_views) {
        vkDestroyImageView(data->logical_device, image_view, nullptr);
    }
    vkDestroySwapchainKHR(data->logical_device, data->swap_chain, nullptr);
}

void recreate_swap_chain(vulkan_data* data, GLFWwindow* window)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    vkDeviceWaitIdle(data->logical_device);
    cleanup_swap_chain(data);
    
    create_swap_chain(data, width, height);
    create_swap_chain_image_views(data);
    create_render_pass(data);
    for (size_t i = 0; i < data->registered_pipelines.size(); i++) {
        auto p = data->registered_pipelines[i];
        p->terminate(*data);
        p->initialise(*data, data->render_pass);
    }
    create_frame_buffers(data);
    for (size_t i = 0; i < data->registered_command_buffers.size(); i++) {
        data->registered_command_buffers[i]->recreate(*data);
    }
}

void initialise_vulkan(vulkan_data* data, GLFWwindow* window)
{
    data->glfw_window = window;
    std::vector<const char*> required_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    create_instance(data);
    create_surface(data, window);
    pick_physical_device(data, required_extensions);
    create_logical_device(data, required_extensions);
    initialise_memory_allocator(data);
    create_semaphores(data, data->image_available_sems);
    create_semaphores(data, data->render_finished_sems);
    create_fences(data, data->in_flight_fences);
    create_command_pools(data);

    create_swap_chain(data, width, height);
    create_swap_chain_image_views(data);
    create_render_pass(data);
    for (size_t i = 0; i < data->registered_pipelines.size(); i++) {
        data->registered_pipelines[i]->initialise(*data, data->render_pass);
    }
    create_frame_buffers(data);
    for (size_t i = 0; i < data->registered_command_buffers.size(); i++) {
        data->registered_command_buffers[i]->recreate(*data);
    }
}

void terminate_vulkan(vulkan_data& data)
{
    vkDeviceWaitIdle(data.logical_device);

    cleanup_swap_chain(&data);
    vkDestroyCommandPool(data.logical_device, data.command_pool_graphics, nullptr);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(data.logical_device, data.image_available_sems[i], nullptr);
        vkDestroySemaphore(data.logical_device, data.render_finished_sems[i], nullptr);
        vkDestroyFence(data.logical_device, data.in_flight_fences[i], nullptr);
    }
    terminate_memory_allocator(data);
    vkDestroyDevice(data.logical_device, nullptr);
    vkDestroySurfaceKHR(data.instance, data.surface, nullptr);
    vkDestroyInstance(data.instance, nullptr);
    data.instance = nullptr;
    data.glfw_window = nullptr;
}

bool is_vulkan_initialised(vulkan_data& data)
{
    return (data.instance != nullptr);
}

VkShaderModule create_shader_module_from_spirv(vulkan_data& vulkan, std::vector<char>& shader_data)
{
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = shader_data.size();
    create_info.pCode = reinterpret_cast<const uint32_t*> (shader_data.data());

    VkShaderModule module;
    if (vkCreateShaderModule(vulkan.logical_device, &create_info, nullptr, &module) != VK_SUCCESS) {
        throw std::runtime_error("Unable to load shader module!");
    }
    return module;
}

VkPipelineShaderStageCreateInfo gen_shader_stage_create_info(VkShaderModule module, shader_type type, const char* entry_point)
{
    VkShaderStageFlagBits stage_flag;
    switch (type) {
        case shader_type::VERTEX:    {stage_flag = VK_SHADER_STAGE_VERTEX_BIT;}     break;
        case shader_type::FRAGMENT:  {stage_flag = VK_SHADER_STAGE_FRAGMENT_BIT;}   break;
        case shader_type::GEOMETRY:  {stage_flag = VK_SHADER_STAGE_GEOMETRY_BIT;}   break;
        case shader_type::COMPUTE:   {stage_flag = VK_SHADER_STAGE_COMPUTE_BIT;}    break;
        default:                     throw std::runtime_error("What...");    break;
    }
    VkPipelineShaderStageCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    create_info.stage = stage_flag;
    create_info.module = module;
    create_info.pName = entry_point;

    return create_info;
}

void submit_command_buffers_graphics(vulkan_data& data, std::vector<VkCommandBuffer> command_buffers)
{
    vkWaitForFences(data.logical_device, 1, &data.in_flight_fences[data.current_frame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = get_image_index(data);
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {data.image_available_sems[data.current_frame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffers[imageIndex];
    VkSemaphore signalSemaphores[] = {data.render_finished_sems[data.current_frame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(data.logical_device, 1, &data.in_flight_fences[data.current_frame]);
    if (vkQueueSubmit(data.graphics_queue, 1, &submitInfo, data.in_flight_fences[data.current_frame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

void present_frame(vulkan_data& data)
{
    VkSemaphore signalSemaphores[] = {data.render_finished_sems[data.current_frame]};
    VkSwapchainKHR swapChains[] = {data.swap_chain};
    uint32_t imageIndex = get_image_index(data);

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1; 
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    auto result = vkQueuePresentKHR(data.present_queue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreate_swap_chain(&data, data.glfw_window);
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
    data.current_frame = ((data.current_frame + 1) % MAX_FRAMES_IN_FLIGHT);
    data.image_index = -1;
}

template <typename T>
bool vector_contains(T value, std::vector<T>& vector, size_t* index)
{
    for (size_t i = 0; i < vector.size(); i++) {
        if (vector[i] == value) {
            *index = i;
            return true;
        }
    }
    return false;
}

void register_command_buffer(vulkan_data& data, graphics_command_buffer* buffer)
{
    if (!vector_contains<graphics_command_buffer*>(buffer, data.registered_command_buffers, nullptr)) {
        data.registered_command_buffers.push_back(buffer);
    }
}

void unregister_command_buffer(vulkan_data& data, graphics_command_buffer* buffer)
{
    size_t index;
    if (vector_contains<graphics_command_buffer*>(buffer, data.registered_command_buffers, &index)) {
        data.registered_command_buffers.erase(data.registered_command_buffers.begin() + index);
    }
}

void register_pipeline(vulkan_data& data, graphics_pipeline* pipeline)
{
    if (!vector_contains<graphics_pipeline*>(pipeline, data.registered_pipelines, nullptr)) {
        data.registered_pipelines.push_back(pipeline);
    }
}

void unregister_pipeline(vulkan_data& data, graphics_pipeline* pipeline)
{
    size_t index;
    if (vector_contains<graphics_pipeline*>(pipeline, data.registered_pipelines, &index)) {
        data.registered_pipelines.erase(data.registered_pipelines.begin() + index);
    }
}

void create_buffer(vulkan_data& data, VkBuffer* buffer, VmaAllocation* allocation, VkDeviceSize byte_data_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = byte_data_size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memory_usage;
    
    if (vmaCreateBuffer(data.mem_allocator, &bufferInfo, &allocInfo, buffer, allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }
}

uint32_t get_image_index(vulkan_data& data)
{
    uint32_t image_index;
    if (data.image_index < 0) {
        vkAcquireNextImageKHR(data.logical_device, data.swap_chain, UINT64_MAX, data.image_available_sems[data.current_frame], VK_NULL_HANDLE, &image_index);
        data.image_index = image_index;
    } else {
        image_index = (uint32_t)data.image_index;
    }
    return image_index;
}

/* https://vulkan-tutorial.com/Uniform_buffers/Descriptor_pool_and_sets - Descriptor pool and sets */