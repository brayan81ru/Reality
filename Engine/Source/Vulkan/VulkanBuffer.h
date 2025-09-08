#pragma once

#include "GraphicsDevice.h"
#include "Resource.h"
#include <vulkan/vulkan.h>

namespace Reality {
    class VulkanDevice;

    class VulkanBuffer : public BufferBase {
    private:
        VkBuffer m_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_bufferMemory = VK_NULL_HANDLE;
        VulkanDevice* m_device;
        void* m_mappedData = nullptr;

    public:
        VulkanBuffer(VulkanDevice* device, const BufferDesc& desc, const void* initialData = nullptr);
        ~VulkanBuffer();

        // IBuffer interface
        void* Map() override;
        void Unmap() override;
        void UpdateData(const void* data, size_t size, size_t offset = 0) override;

        // Helper methods
        VkBuffer GetBuffer() const { return m_buffer; }
        VkDeviceMemory GetMemory() const { return m_bufferMemory; }
    };
}
