#pragma once

#include "GraphicsDevice.h"
#include "Resource.h"
#include <vulkan/vulkan.h>

namespace Reality {
    class VulkanDevice;

    class VulkanTexture : public TextureBase {
    private:
        VkImage m_image = VK_NULL_HANDLE;
        VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
        VkImageView m_imageView = VK_NULL_HANDLE;
        VkSampler m_sampler = VK_NULL_HANDLE;
        VulkanDevice* m_device;
        bool m_ownsImage;
        
    public:
        // Create from description
        VulkanTexture(VulkanDevice* device, const TextureDesc& desc, const void* initialData = nullptr);
        
        // Create from existing image (e.g., swap chain image)
        VulkanTexture(VulkanDevice* device, VkImage image, VkImageView imageView, VkFormat format);
        
        ~VulkanTexture();
        
        // ITexture interface
        void UpdateData(const void* data, uint32_t mipLevel, uint32_t arraySlice) override;
        
        // Helper methods
        VkImage GetImage() const { return m_image; }
        VkDeviceMemory GetMemory() const { return m_imageMemory; }
        VkImageView GetImageView() const { return m_imageView; }
        VkSampler GetSampler() const { return m_sampler; }
    };
}