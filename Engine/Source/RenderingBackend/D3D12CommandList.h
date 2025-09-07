#pragma once

#include "GraphicsDevice.h"
#include "Resource.h"
#include <d3d12.h>
#include <wrl/client.h>
#include <vector>

namespace Reality {
    class D3D12Device;

    class D3D12CommandList : public CommandListBase {
    private:
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
        D3D12Device* m_device;
        std::vector<D3D12_RESOURCE_BARRIER> m_barriers;

    public:
        D3D12CommandList(D3D12Device* device);
        ~D3D12CommandList() = default;

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
        ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList.Get(); }

    protected:
        void ResetImpl() override;
        void CloseImpl() override;
    };
}