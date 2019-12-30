#include "vulkan_base.h"

class graphics_command_buffer
{
private:
    std::vector<VkCommandBuffer> command_buffers;

public:
    uint32_t flags = 0x00;
    VkCommandBufferLevel buffer_level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    void initialise(vulkan_data& data);
    void recreate(vulkan_data& data);
    void terminate(vulkan_data& data);
    std::vector<VkCommandBuffer>& cmd_buffers() {return command_buffers;}

protected:
    virtual void fill_command_buffer(vulkan_data& data, VkCommandBuffer& buffer, VkFramebuffer& frame_buffer) = 0;
    
};