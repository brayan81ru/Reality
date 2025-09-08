#pragma once

#include "GraphicsDevice.h"
#include "Resource.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace Reality {
    class VulkanDevice;

    class VulkanCommandList : public CommandListBase {
    private:
        VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
        VulkanDevice* m_device;
        bool m_isRecording = false;
        
        std::vector<VkBuffer> m_vertexBuffers;
        VkBuffer m_indexBuffer = VK_NULL_HANDLE;
        VkPipeline m_currentPipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_currentPipelineLayout = VK_NULL_HANDLE;
        std::vector<VkViewport> m_viewports;
        std::vector<VkRect2D> m_scissors;
        
    public:
        VulkanCommandList(VulkanDevice* device);
        ~VulkanCommandList() = default;
        
        // ICommandList interface
        void ResourceBarrier(ITexture* resource, ResourceState before, ResourceState after) override;
        void SetPipelineState(IPipelineState* pipeline) override;
        void SetVertexBuffers(IBuffer* const* buffers, uint32_t startSlot, uint32_t numBuffers) override;
        void SetIndexBuffer(IBuffer* buffer) override;
        void SetGraphicsRootConstantBufferView(uint32_t rootIndex, IBuffer* buffer) override;
        void SetGraphicsRootDescriptorTable(uint32_t rootIndex, IBuffer* buffer) override;
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1) override;
        void CopyTextureRegion(ITexture* dst, ITexture* src) override;
        void ClearRenderTargetView(ITexture* renderTarget, const float color[4]) override;
        void ClearDepthStencilView(ITexture* depthStencil, float depth, uint8_t stencil) override;
        void OMSetRenderTargets(uint32_t numRenderTargets, ITexture* const* renderTargets, ITexture* depthStencil) override;
        void RSSetViewports(uint32_t numViewports, const Viewport* viewports) override;
        void RSSetScissorRects(uint32_t numRects, const Rect* rects) override;
        
        // Helper methods
        VkCommandBuffer GetCommandBuffer() const { return m_commandBuffer; }
        
    protected:
        void ResetImpl() override;
        void CloseImpl() override;
    };
}