#include "vulkan_base.h"

#include "../platform.h"

#include <fstream>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"


struct physical_device_indices
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

    uint32_t numGlfwExt = 0;
    auto glfwExt = glfwGetRequiredInstanceExtensions(&numGlfwExt);
    std::vector<const char*> extensions(glfwExt, glfwExt + numGlfwExt);

#ifndef NDEBUG
    /* extensions */
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

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

    create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

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

physical_device_indices get_device_indices(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    physical_device_indices indices{};

    uint32_t num_queue_families = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &num_queue_families, nullptr);
    std::vector<VkQueueFamilyProperties> queue_family_properties(num_queue_families);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &num_queue_families, queue_family_properties.data());
    int index = 0;
    for (VkQueueFamilyProperties prop : queue_family_properties) {
        if (prop.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.has_graphics_queue = true;
            indices.graphics_queue_index = index;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &present_support);
        if (present_support) {
            indices.has_present_queue = true;
            indices.present_queue_index = index;
        }

        index++;
    }

    return indices;
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
    auto indices = get_device_indices(device, surface);
    bool ext = check_device_extension_support(device, required_extensions);
    bool swap_chain_is_adequate = false;
    if (ext) {
        auto details = query_swap_chain_support(device, surface);
        swap_chain_is_adequate = !details.formats.empty() && !details.present_modes.empty();
    }
    return indices.has_graphics_queue && ext && swap_chain_is_adequate;
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
    std::vector<VkPhysicalDevice> suitable_devices;

    // find all suitable devices (required extensions)
    for (VkPhysicalDevice device : devices) {
        if (is_device_suitable(device, data->surface, required_extensions)) {
            suitable_devices.push_back(device);
            data->physical_device = device;
            break;
        }
    }

    // score suitable devices for best physical device (optional features)
    VkPhysicalDevice* best_device = VK_NULL_HANDLE;
    int best_score = 0;
    for (VkPhysicalDevice device: suitable_devices) {
        int score = 0;
        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        if (supportedFeatures.samplerAnisotropy) {
            score += 1;
        }

        if (score > best_score) {
            best_device = &device;
            best_score = score;
        }
    }

    data->physical_device = *best_device;
    if (data->physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("Unable to select a suitable physical device!");
    }
}

void create_logical_device(vulkan_data* data, std::vector<const char*>& required_extensions)
{
    auto queue_indices = get_device_indices(data->physical_device, data->surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::vector<uint32_t> uniqueQueueFamilies = {queue_indices.graphics_queue_index, queue_indices.present_queue_index};

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

    vkGetDeviceQueue(data->logical_device, queue_indices.graphics_queue_index, 0, &data->graphics_queue);
    vkGetDeviceQueue(data->logical_device, queue_indices.present_queue_index, 0, &data->present_queue);
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
    auto indices = get_device_indices(data->physical_device, data->surface);
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

VkFormat find_supported_depth_format(vulkan_data& data, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(data.physical_device, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("Unable to find a suitable depth format!");
    // @TODO: handle this without throwing a error?, maybe revert to linear if optimal does not exist?
}

VkFormat find_depth_format(vulkan_data& data) {
    return find_supported_depth_format(data,
                                       {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                       VK_IMAGE_TILING_OPTIMAL,
                                       VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void create_render_pass(vulkan_data* data)
{
    // @TODO this will have to be updated in the future for post processing etc.
    // @TODO multisampling
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

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = find_depth_format(*data);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
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
        std::array<VkImageView, 2> attachments = {
            data->swap_chain_data.image_views[i],
            data->depth_resources.image_view
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = data->render_pass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
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
    auto indicies = get_device_indices(data->physical_device, data->surface);

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

void create_depth_resources(vulkan_data* data) {
    VkFormat depth_format = find_depth_format(*data);

    // create depth image
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = data->swap_chain_data.extent.width;
    imageInfo.extent.height = data->swap_chain_data.extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = depth_format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0; // Optional

    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(data->mem_allocator, &imageInfo, &allocationCreateInfo, &data->depth_resources.image, &data->depth_resources.image_allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create depth image!");
    }

    // create depth image view
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = data->depth_resources.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = depth_format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(data->logical_device, &viewInfo, nullptr, &data->depth_resources.image_view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create depth image view!");
    }
}

void cleanup_depth_resources(vulkan_data* data) {
    vkDestroyImageView(data->logical_device, data->depth_resources.image_view, nullptr);
    vmaDestroyImage(data->mem_allocator, data->depth_resources.image, data->depth_resources.image_allocation);
}

void cleanup_swap_chain(vulkan_data* data)
{
    for (auto framebuffer : data->swap_chain_data.frame_buffers) {
        vkDestroyFramebuffer(data->logical_device, framebuffer, nullptr);
    }
    cleanup_depth_resources(data);
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

    // cleanup
    for (graphics_command_buffer* buffer : data->registered_command_buffers) {
        buffer->reterminate(*data);
    }
    for (graphics_pipeline* pipeline : data->registered_pipelines) {
        pipeline->reterminate(*data);
    }
    cleanup_swap_chain(data);

    // recreate
    create_swap_chain(data, width, height);
    create_swap_chain_image_views(data);
    create_render_pass(data);
    create_depth_resources(data);
    create_frame_buffers(data);
    for (graphics_pipeline* pipeline : data->registered_pipelines) {
        pipeline->reinitialise(*data, data->render_pass);
    }
    for (graphics_command_buffer* buffer : data->registered_command_buffers) {
        buffer->reinitialise(*data);
    }
}

void create_defaults(vulkan_data* data) {
    data->default_image = new vulkan_image;
    data->default_image->initialise_default(*data);
}

inline bool depth_format_has_stencil_component(const VkFormat& format) {
    return (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT);
}

void terminate_defaults(vulkan_data& data) {
    data.default_image->terminate(data);
    delete data.default_image;
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
    create_depth_resources(data);
    create_frame_buffers(data);
    create_defaults(data);
}

void terminate_vulkan(vulkan_data& data)
{
    vkDeviceWaitIdle(data.logical_device);

    terminate_defaults(data);
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
    auto err = vkQueueSubmit(data.graphics_queue, 1, &submitInfo, data.in_flight_fences[data.current_frame]);
    if (err != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!  error id: " + std::to_string(err));
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

template <typename T>
void register_struct(std::vector<T*>& registered_objs, T* obj)
{
    if (!vector_contains<T*>(obj, registered_objs, nullptr)) {
        registered_objs.push_back(obj);
    }
}

template <typename T>
void unregister_struct(std::vector<T*>& registered_objs, T* obj)
{
    size_t index;
    if (vector_contains<T*>(obj, registered_objs, &index)) {
        registered_objs.erase(registered_objs.begin() + index);
    }
}

void register_command_buffer(vulkan_data& data, graphics_command_buffer* buffer)
{
    register_struct<graphics_command_buffer>(data.registered_command_buffers, buffer);
}

void unregister_command_buffer(vulkan_data& data, graphics_command_buffer* buffer)
{
    unregister_struct<graphics_command_buffer>(data.registered_command_buffers, buffer);
}

void register_pipeline(vulkan_data& data, graphics_pipeline* pipeline)
{
    register_struct<graphics_pipeline>(data.registered_pipelines, pipeline);
}

void unregister_pipeline(vulkan_data& data, graphics_pipeline* pipeline)
{
    unregister_struct<graphics_pipeline>(data.registered_pipelines, pipeline);
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

VkPhysicalDeviceFeatures get_device_features(vulkan_data& data)
{
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(data.physical_device, &supportedFeatures);
    return supportedFeatures;
}

void fill_buffer(vulkan_data& vkdata, VmaAllocation& alloc, size_t data_length, void* data_start)
{
    void* mapped_mem;
    vmaMapMemory(vkdata.mem_allocator, alloc, &mapped_mem);
    memcpy(mapped_mem, data_start, data_length);
    vmaUnmapMemory(vkdata.mem_allocator, alloc);
}
