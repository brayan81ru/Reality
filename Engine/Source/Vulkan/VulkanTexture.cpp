#include "VulkanTexture.h"
#include "VulkanDevice.h"
#include <cassert>

namespace Reality {
    VulkanTexture::VulkanTexture(VulkanDevice* device, const TextureDesc& desc, const void* initialData)
        : TextureBase(desc), m_device(device), m_ownsImage(true) {
        VkDevice vkDevice = device->GetDevice();

        // Create image
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

        switch (desc.type) {
            case ResourceType::Texture1D:
                imageInfo.imageType = VK_IMAGE_TYPE_1D;
                break;
            case ResourceType::Texture2D:
                imageInfo.imageType = VK_IMAGE_TYPE_2D;
                break;
            case ResourceType::Texture3D:
                imageInfo.imageType = VK_IMAGE_TYPE_3D;
                break;
            case ResourceType::TextureCube:
                imageInfo.imageType = VK_IMAGE_TYPE_2D;
                imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
                break;
            default:
                assert(false && "Unknown resource type");
                return;
        }

        imageInfo.extent.width = desc.width;
        imageInfo.extent.height = desc.height;
        imageInfo.extent.depth = desc.depth;
        imageInfo.mipLevels = desc.mipLevels;
        imageInfo.arrayLayers = desc.arraySize;
        imageInfo.format = static_cast<VkFormat>(desc.format);
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

        // Set usage flags based on bind flags
        if (desc.bindFlags & TextureBindFlags::RenderTarget) {
            imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
        if (desc.bindFlags & TextureBindFlags::DepthStencil) {
            imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        if (desc.bindFlags & TextureBindFlags::UnorderedAccess) {
            imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
        }

        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if (vkCreateImage(vkDevice, &imageInfo, nullptr, &m_image) != VK_SUCCESS) {
            assert(false && "Failed to create image");
            return;
        }

        // Get memory requirements
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(vkDevice, m_image, &memRequirements);

        // Find memory type
        uint32_t memoryTypeIndex = device->FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // Allocate memory
        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;

        if (vkAllocateMemory(vkDevice, &allocInfo, nullptr, &m_imageMemory) != VK_SUCCESS) {
            assert(false && "Failed to allocate image memory");
            return;
        }

        // Bind memory
        vkBindImageMemory(vkDevice, m_image, m_imageMemory, 0);

        // Transition image layout
        device->TransitionImageLayout(m_image, static_cast<VkFormat>(desc.format), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // Set initial data if provided
        if (initialData) {
            // Create staging buffer
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;

            VkBufferCreateInfo stagingBufferInfo = {};
            stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            stagingBufferInfo.size = desc.width * desc.height * 4; // Assuming 4 bytes per pixel
            stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateBuffer(vkDevice, &stagingBufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS) {
                assert(false && "Failed to create staging buffer");
                return;
            }

            // Get memory requirements for staging buffer
            VkMemoryRequirements stagingMemRequirements;
            vkGetBufferMemoryRequirements(vkDevice, stagingBuffer, &stagingMemRequirements);

            // Allocate memory for staging buffer
            VkMemoryAllocateInfo stagingAllocInfo = {};
            stagingAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            stagingAllocInfo.allocationSize = stagingMemRequirements.size;
            stagingAllocInfo.memoryTypeIndex = device->FindMemoryType(stagingMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            if (vkAllocateMemory(vkDevice, &stagingAllocInfo, nullptr, &stagingBufferMemory) != VK_SUCCESS) {
                assert(false && "Failed to allocate staging buffer memory");
                return;
            }

            // Bind memory to staging buffer
            vkBindBufferMemory(vkDevice, stagingBuffer, stagingBufferMemory, 0);

            // Map staging buffer and copy data
            void* data;
            vkMapMemory(vkDevice, stagingBufferMemory, 0, stagingBufferInfo.size, 0, &data);
            memcpy(data, initialData, stagingBufferInfo.size);
            vkUnmapMemory(vkDevice, stagingBufferMemory);

            // Copy from staging buffer to image
            device->CopyBufferToImage(stagingBuffer, m_image, desc.width, desc.height);

            // Transition image layout for shader access
            device->TransitionImageLayout(m_image, static_cast<VkFormat>(desc.format), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            // Cleanup staging buffer
            vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
            vkFreeMemory(vkDevice, stagingBufferMemory, nullptr);
        }

        // Create image view
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_image;

        switch (desc.type) {
            case ResourceType::Texture1D:
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
                break;
            case ResourceType::Texture2D:
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                break;
            case ResourceType::Texture3D:
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
                break;
            case ResourceType::TextureCube:
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
                break;
            default:
                assert(false && "Unknown resource type");
                return;
        }

        viewInfo.format = static_cast<VkFormat>(desc.format);
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = desc.mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = desc.arraySize;

        if (vkCreateImageView(vkDevice, &viewInfo, nullptr, &m_imageView) != VK_SUCCESS) {
            assert(false && "Failed to create image view");
            return;
        }

        // Create sampler
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 16.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(desc.mipLevels);

        if (vkCreateSampler(vkDevice, &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
            assert(false && "Failed to create sampler");
            return;
        }
    }

    VulkanTexture::VulkanTexture(VulkanDevice* device, VkImage image, VkImageView imageView, VkFormat format)
        : TextureBase(TextureDesc()), m_device(device), m_ownsImage(false) {
        m_image = image;
        m_imageView = imageView;
        m_desc.format = static_cast<Format>(format);

        // Fill in texture description from image
        // This is a simplified version; a more complete implementation would query the image properties
        m_desc.type = ResourceType::Texture2D;
        m_desc.width = 800; // Placeholder - should be queried
        m_desc.height = 600; // Placeholder - should be queried
        m_desc.depth = 1;
        m_desc.mipLevels = 1;
        m_desc.arraySize = 1;
    }

    VulkanTexture::~VulkanTexture() {
        VkDevice vkDevice = m_device->GetDevice();

        if (m_sampler != VK_NULL_HANDLE) {
            vkDestroySampler(vkDevice, m_sampler, nullptr);
        }

        if (m_imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(vkDevice, m_imageView, nullptr);
        }

        if (m_ownsImage) {
            if (m_image != VK_NULL_HANDLE) {
                vkDestroyImage(vkDevice, m_image, nullptr);
            }
            if (m_imageMemory != VK_NULL_HANDLE) {
                vkFreeMemory(vkDevice, m_imageMemory, nullptr);
            }
        }
    }

    void VulkanTexture::UpdateData(const void* data, uint32_t mipLevel, uint32_t arraySlice) {
        // Implementation for updating texture data
        // This would involve creating a staging buffer and copying to the texture
        // For now, we'll just assert that it's not implemented
        assert(false && "UpdateData for textures not implemented yet");
    }
}