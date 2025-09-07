#pragma once
#include "GraphicsDevice.h"
#include <vector>
#include "Resource.h"

namespace Reality {
    // Command List Implementation
    class CommandList : public CommandListBase {
    private:
        std::vector<ITexture*> m_renderTargets;
        ITexture* m_depthStencil = nullptr;
        std::vector<Viewport> m_viewports;
        std::vector<Rect> m_scissorRects;
        IPipelineState* m_currentPipeline = nullptr;
        std::vector<IBuffer*> m_vertexBuffers;
        IBuffer* m_indexBuffer = nullptr;

    public:
        CommandList() = default;
        virtual ~CommandList() = default;

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

    protected:
        void ResetImpl() override;
        void CloseImpl() override;
    };
}
