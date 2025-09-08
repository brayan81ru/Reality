#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include <cassert>

#include "DiligentCore/ThirdParty/Vulkan-Headers/include/vulkan/vulkan_core.h"

namespace Reality {
    VulkanBuffer::VulkanBuffer(VulkanDevice* device, const BufferDesc& desc, const void* initialData)
        : BufferBase(desc), m_device(device) {
        VkDevice vkDevice = device->GetDevice();
        VkPhysicalDevice physicalDevice = device->GetPhysicalDevice();
        
        // Create buffer
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = desc.size;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        
        // Set usage flags based on bind flags
        if (desc.bindFlags & BufferBindFlags::VertexBuffer) {
            bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }
        if (desc.bindFlags & BufferBindFlags::IndexBuffer) {
            bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }
        if (desc.bindFlags & BufferBindFlags::ConstantBuffer) {
            bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }
        if (desc.bindFlags & BufferBindFlags::ShaderResource) {
            bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }
        if (desc.bindFlags & BufferBindFlags::UnorderedAccess) {
            bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }
        
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        if (vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &m_buffer) != VK_SUCCESS) {
            assert(false && "Failed to create buffer");
            return;
        }
        
        // Get memory requirements
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(vkDevice, m_buffer, &memRequirements);
        
        // Find memory type
        VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        if (desc.usage == ResourceUsage::Dynamic) {
            properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        }
        
        uint32_t memoryTypeIndex = device->FindMemoryType(memRequirements.memoryTypeBits, properties);
        
        // Allocate memory
        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;
        
        if (vkAllocateMemory(vkDevice, &allocInfo, nullptr, &m_bufferMemory) != VK_SUCCESS) {
            assert(false && "Failed to allocate buffer memory");
            return;
        }
        
        // Bind memory
        vkBindBufferMemory(vkDevice, m_buffer, m_bufferMemory, 0);
        
        // Set initial data if provided
        if (initialData) {
            if (desc.usage == ResourceUsage::Dynamic) {
                // For dynamic resources, we can map and copy directly
                void* mappedData = Map();
                memcpy(mappedData, initialData, desc.size);
                Unmap();
            } else {
                // For default resources, we need to use a staging buffer
                VkBuffer stagingBuffer;
                VkDeviceMemory stagingBufferMemory;
                
                // Create staging buffer
                VkBufferCreateInfo stagingBufferInfo = {};
                stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                stagingBufferInfo.size = desc.size;
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
                vkMapMemory(vkDevice, stagingBufferMemory, 0, desc.size, 0, &data);
                memcpy(data, initialData, desc.size);
                vkUnmapMemory(vkDevice, stagingBufferMemory);
                
                // Copy from staging buffer to device buffer
                device->CopyBuffer(stagingBuffer, m_buffer, desc.size);
                
                // Cleanup staging buffer
                vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
                vkFreeMemory(vkDevice, stagingBufferMemory, nullptr);
            }
        }
    }
    
    VulkanBuffer::~VulkanBuffer() {
        if (m_mappedData) {
            Unmap();
        }
        
        VkDevice vkDevice = m_device->GetDevice();
        if (m_buffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(vkDevice, m_buffer, nullptr);
        }
        if (m_bufferMemory != VK_NULL_HANDLE) {
            vkFreeMemory(vkDevice, m_bufferMemory, nullptr);
        }
    }
    
    void* VulkanBuffer::Map() {
        if (!m_mappedData) {
            VkDevice vkDevice = m_device->GetDevice();
            if (vkMapMemory(vkDevice, m_bufferMemory, 0, m_desc.size, 0, &m_mappedData) != VK_SUCCESS) {
                return nullptr;
            }
        }
        return m_mappedData;
    }
    
    void VulkanBuffer::Unmap() {
        if (m_mappedData) {
            VkDevice vkDevice = m_device->GetDevice();
            vkUnmapMemory(vkDevice, m_bufferMemory);
            m_mappedData = nullptr;
        }
    }
    
    void VulkanBuffer::UpdateData(const void* data, size_t size, size_t offset) {
        void* mappedData = Map();
        if (mappedData) {
            memcpy(static_cast<char*>(mappedData) + offset, data, size);
            Unmap();
        }
    }
}
