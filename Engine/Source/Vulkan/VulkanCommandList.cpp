#include "VulkanCommandList.h"
#include "VulkanDevice.h"
#include "VulkanTexture.h"
#include "VulkanBuffer.h"
#include "VulkanPipelineState.h"
#include <cassert>

namespace Reality {
    VulkanCommandList::VulkanCommandList(VulkanDevice* device) : m_device(device) {
        VkDevice vkDevice = device->GetDevice();
        VkCommandPool commandPool = device->GetCommandPool();
        
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        
        if (vkAllocateCommandBuffers(vkDevice, &allocInfo, &m_commandBuffer) != VK_SUCCESS) {
            assert(false && "Failed to allocate command buffer");
            return;
        }
        
        // Start in a closed state
        m_isClosed = true;
    }
    
    void VulkanCommandList::ResourceBarrier(ITexture* resource, ResourceState before, ResourceState after) {
        assert(!m_isClosed && "Command list is closed");
        
        VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(resource);
        VkImage image = vulkanTexture->GetImage();
        
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = static_cast<VkImageLayout>(before);
        barrier.newLayout = static_cast<VkImageLayout>(after);
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        
        vkCmdPipelineBarrier(m_commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }
    
    void VulkanCommandList::SetPipelineState(IPipelineState* pipeline) {
        assert(!m_isClosed && "Command list is closed");
        
        VulkanPipelineState* vulkanPipeline = static_cast<VulkanPipelineState*>(pipeline);
        m_currentPipeline = vulkanPipeline->GetPipeline();
        m_currentPipelineLayout = vulkanPipeline->GetLayout();
        
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_currentPipeline);
    }
    
    void VulkanCommandList::SetVertexBuffers(IBuffer* const* buffers, uint32_t startSlot, uint32_t numBuffers) {
        assert(!m_isClosed && "Command list is closed");
        
        // Ensure we have enough space in the vertex buffers array
        if (startSlot + numBuffers > m_vertexBuffers.size()) {
            m_vertexBuffers.resize(startSlot + numBuffers);
        }
        
        // Set the vertex buffers
        for (uint32_t i = 0; i < numBuffers; i++) {
            VulkanBuffer* vulkanBuffer = static_cast<VulkanBuffer*>(buffers[i]);
            m_vertexBuffers[startSlot + i] = vulkanBuffer->GetBuffer();
        }
        
        // Bind vertex buffers
        std::vector<VkDeviceSize> offsets(numBuffers, 0);
        vkCmdBindVertexBuffers(m_commandBuffer, startSlot, numBuffers, &m_vertexBuffers[startSlot], offsets.data());
    }
    
    void VulkanCommandList::SetIndexBuffer(IBuffer* buffer) {
        assert(!m_isClosed && "Command list is closed");
        
        VulkanBuffer* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
        m_indexBuffer = vulkanBuffer->GetBuffer();
        
        vkCmdBindIndexBuffer(m_commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    }
    
    void VulkanCommandList::SetGraphicsRootConstantBufferView(uint32_t rootIndex, IBuffer* buffer) {
        assert(!m_isClosed && "Command list is closed");
        
        VulkanBuffer* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
        VkBuffer vkBuffer = vulkanBuffer->GetBuffer();
        VkDeviceSize offset = 0;
        
        vkCmdBindVertexBuffers(m_commandBuffer, rootIndex, 1, &vkBuffer, &offset);
    }
    
    void VulkanCommandList::SetGraphicsRootDescriptorTable(uint32_t rootIndex, IBuffer* buffer) {
        assert(!m_isClosed && "Command list is closed");
        // Implementation for descriptor tables would go here
        // For now, we'll just assert that it's not implemented
        assert(false && "SetGraphicsRootDescriptorTable not implemented yet");
    }
    
    void VulkanCommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount) {
        assert(!m_isClosed && "Command list is closed");
        assert(m_currentPipeline != VK_NULL_HANDLE && "No pipeline state set");
        assert(m_indexBuffer != VK_NULL_HANDLE && "No index buffer set");
        
        vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, 0, 0, 0);
    }
    
    void VulkanCommandList::CopyTextureRegion(ITexture* dst, ITexture* src) {
        assert(!m_isClosed && "Command list is closed");
        
        VulkanTexture* vulkanDst = static_cast<VulkanTexture*>(dst);
        VulkanTexture* vulkanSrc = static_cast<VulkanTexture*>(src);
        
        VkImageCopy imageCopyRegion = {};
        imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.srcSubresource.layerCount = 1;
        imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.dstSubresource.layerCount = 1;
        imageCopyRegion.extent.width = vulkanDst->GetWidth();
        imageCopyRegion.extent.height = vulkanDst->GetHeight();
        imageCopyRegion.extent.depth = 1;
        
        vkCmdCopyImage(m_commandBuffer, vulkanSrc->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
                        vulkanDst->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
    }
    
    void VulkanCommandList::ClearRenderTargetView(ITexture* renderTarget, const float color[4]) {
        assert(!m_isClosed && "Command list is closed");
        
        VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(renderTarget);
        
        VkClearColorValue clearColor = {};
        clearColor.float32[0] = color[0];
        clearColor.float32[1] = color[1];
        clearColor.float32[2] = color[2];
        clearColor.float32[3] = color[3];
        
        VkImageSubresourceRange range = {};
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;
        
        vkCmdClearColorImage(m_commandBuffer, vulkanTexture->GetImage(), VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &range);
    }
    
    void VulkanCommandList::ClearDepthStencilView(ITexture* depthStencil, float depth, uint8_t stencil) {
        assert(!m_isClosed && "Command list is closed");
        
        VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(depthStencil);
        
        VkClearDepthStencilValue clearValue = {};
        clearValue.depth = depth;
        clearValue.stencil = stencil;
        
        VkImageSubresourceRange range = {};
        range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;
        
        vkCmdClearDepthStencilImage(m_commandBuffer, vulkanTexture->GetImage(), VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &range);
    }
    
    void VulkanCommandList::OMSetRenderTargets(uint32_t numRenderTargets, ITexture* const* renderTargets, ITexture* depthStencil) {
        assert(!m_isClosed && "Command list is closed");
        
        // This would typically be done at the beginning of a render pass in Vulkan
        // For now, we'll just assert that it's not implemented
        assert(false && "OMSetRenderTargets not fully implemented yet");
    }
    
    void VulkanCommandList::RSSetViewports(uint32_t numViewports, const Viewport* viewports) {
        assert(!m_isClosed && "Command list is closed");
        
        m_viewports.resize(numViewports);
        for (uint32_t i = 0; i < numViewports; i++) {
            m_viewports[i] = {
                viewports[i].x,
                viewports[i].y,
                viewports[i].width,
                viewports[i].height,
                viewports[i].minDepth,
                viewports[i].maxDepth
            };
        }
        
        vkCmdSetViewport(m_commandBuffer, 0, numViewports, m_viewports.data());
    }
    
    void VulkanCommandList::RSSetScissorRects(uint32_t numRects, const Rect* rects) {
        assert(!m_isClosed && "Command list is closed");
        
        m_scissors.resize(numRects);
        for (uint32_t i = 0; i < numRects; i++) {
            m_scissors[i] = {
                {rects[i].left, rects[i].top},
                {static_cast<uint32_t>(rects[i].right - rects[i].left), static_cast<uint32_t>(rects[i].bottom - rects[i].top)}
            };
        }
        
        vkCmdSetScissor(m_commandBuffer, 0, numRects, m_scissors.data());
    }
    
    void VulkanCommandList::ResetImpl() {
        VkDevice vkDevice = m_device->GetDevice();
        VkCommandPool commandPool = m_device->GetCommandPool();
        
        if (vkResetCommandBuffer(m_commandBuffer, 0) != VK_SUCCESS) {
            assert(false && "Failed to reset command buffer");
            return;
        }
        
        // Reset state
        m_isRecording = false;
        m_vertexBuffers.clear();
        m_indexBuffer = VK_NULL_HANDLE;
        m_currentPipeline = VK_NULL_HANDLE;
        m_currentPipelineLayout = VK_NULL_HANDLE;
        m_viewports.clear();
        m_scissors.clear();
    }
    
    void VulkanCommandList::CloseImpl() {
        if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS) {
            assert(false && "Failed to record command buffer");
            return;
        }
        
        m_isRecording = false;
    }
}