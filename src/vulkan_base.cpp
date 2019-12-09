#include "vulkan_base.h"

struct physical_device_indicies
{
    bool has_graphics_queue = false;
    uint32_t graphics_queue_index = 0;
    bool has_present_queue = false;
    uint32_t present_queue_index = 0;
};

struct swap_chain_support_details
{
    VkSurfaceCapabilitiesKHR capabilities;
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
        "VK_LAYER_KHRONOS_validation"
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
    physical_device_indicies indicies;

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

void initialise_vulkan(vulkan_data* data, GLFWwindow* window)
{
    std::vector<const char*> required_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    create_instance(data);
    create_surface(data, window);
    pick_physical_device(data, required_extensions);
    create_logical_device(data, required_extensions);
    create_swap_chain(data, width, height);
}

void terminate_vulkan(vulkan_data& data)
{
    vkDestroySwapchainKHR(data.logical_device, data.swap_chain, nullptr);
    vkDestroyDevice(data.logical_device, nullptr);
    vkDestroySurfaceKHR(data.instance, data.surface, nullptr);
    vkDestroyInstance(data.instance, nullptr);
}

/* https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Querying-details-of-swap-chain-support - Querying details of swap chain support */