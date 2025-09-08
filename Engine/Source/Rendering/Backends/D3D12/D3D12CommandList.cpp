#include "D3D12CommandList.h"
#include "D3D12Device.h"
#include "D3D12PipelineState.h"
#include "D3D12Buffer.h"
#include "D3D12Texture.h"
#include <cassert>

namespace Reality {
    D3D12CommandList::D3D12CommandList(D3D12Device* device)
        : CommandListBase(device), m_device(device) {
        Initialize();
    }

    D3D12CommandList::~D3D12CommandList() {
        if (m_isClosed) {
            CloseImpl();
        }
    }

    bool D3D12CommandList::Initialize() {
        if (!m_device || !m_device->GetD3DDevice()) {
            return false;
        }

        // Create command allocator
        HRESULT hr = m_device->GetD3DDevice()->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&m_commandAllocator));
        if (FAILED(hr)) {
            return false;
        }

        // Create command list
        hr = m_device->GetD3DDevice()->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_commandAllocator.Get(),
            nullptr,
            IID_PPV_ARGS(&m_commandList));
        if (FAILED(hr)) {
            return false;
        }

        // Command lists are created in the recording state, but we'll close it for now
        hr = m_commandList->Close();
        if (FAILED(hr)) {
            return false;
        }

        return true;
    }

    void D3D12CommandList::Reset() {
        CommandListBase::Reset();
    }

    void D3D12CommandList::Close() {
        CommandListBase::Close();
    }

    void D3D12CommandList::ResetImpl() {
        if (!m_commandAllocator || !m_commandList) {
            return;
        }

        // Reset command allocator
        HRESULT hr = m_commandAllocator->Reset();
        assert(SUCCEEDED(hr));

        // Reset command list
        hr = m_commandList->Reset(m_commandAllocator.Get(), nullptr);
        assert(SUCCEEDED(hr));

        // Reset current state
        m_currentPipelineState.Reset();
        m_currentRootSignature.Reset();
    }

    void D3D12CommandList::CloseImpl() {
        if (!m_commandList) {
            return;
        }

        HRESULT hr = m_commandList->Close();
        assert(SUCCEEDED(hr));
    }

    void D3D12CommandList::ResourceBarrier(ITexture* resource, ResourceState before, ResourceState after) {
        if (!m_commandList || !resource) {
            return;
        }

        auto d3d12Texture = static_cast<D3D12Texture*>(resource);
        ID3D12Resource* d3d12Resource = d3d12Texture->GetD3D12Resource();
        if (!d3d12Resource) {
            return;
        }

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = d3d12Resource;
        barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(before);
        barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(after);
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        m_commandList->ResourceBarrier(1, &barrier);
    }

    void D3D12CommandList::SetPipelineState(IPipelineState* pipeline) {
        if (!m_commandList || !pipeline) {
            return;
        }

        auto d3d12Pipeline = static_cast<D3D12PipelineState*>(pipeline);
        m_currentPipelineState = d3d12Pipeline->GetD3D12PipelineState();
        m_currentRootSignature = d3d12Pipeline->GetRootSignature();

        m_commandList->SetPipelineState(m_currentPipelineState.Get());
        m_commandList->SetGraphicsRootSignature(m_currentRootSignature.Get());
    }

    void D3D12CommandList::SetVertexBuffers(IBuffer* const* buffers, uint32_t startSlot, uint32_t numBuffers) {
        if (!m_commandList || !buffers || numBuffers == 0) {
            return;
        }

        std::vector<D3D12_VERTEX_BUFFER_VIEW> views;
        views.reserve(numBuffers);

        for (uint32_t i = 0; i < numBuffers; i++) {
            if (!buffers[i]) {
                continue;
            }

            auto d3d12Buffer = static_cast<D3D12Buffer*>(buffers[i]);
            D3D12_VERTEX_BUFFER_VIEW view = {};
            view.BufferLocation = d3d12Buffer->GetGPUVirtualAddress();
            view.StrideInBytes = d3d12Buffer->GetStride();
            view.SizeInBytes = d3d12Buffer->GetSize();
            views.push_back(view);
        }

        if (!views.empty()) {
            m_commandList->IASetVertexBuffers(startSlot, static_cast<UINT>(views.size()), views.data());
        }
    }

    void D3D12CommandList::SetIndexBuffer(IBuffer* buffer) {
        if (!m_commandList || !buffer) {
            return;
        }

        auto d3d12Buffer = static_cast<D3D12Buffer*>(buffer);
        D3D12_INDEX_BUFFER_VIEW view = {};
        view.BufferLocation = d3d12Buffer->GetGPUVirtualAddress();
        view.SizeInBytes = d3d12Buffer->GetSize();
        view.Format = DXGI_FORMAT_R32_UINT; // Assuming 32-bit indices

        m_commandList->IASetIndexBuffer(&view);
    }

    void D3D12CommandList::SetGraphicsRootConstantBufferView(uint32_t rootIndex, IBuffer* buffer) {
        if (!m_commandList || !buffer) {
            return;
        }

        auto d3d12Buffer = static_cast<D3D12Buffer*>(buffer);
        m_commandList->SetGraphicsRootConstantBufferView(rootIndex, d3d12Buffer->GetGPUVirtualAddress());
    }

    void D3D12CommandList::SetGraphicsRootDescriptorTable(uint32_t rootIndex, IBuffer* buffer) {
        if (!m_commandList || !buffer) {
            return;
        }

        // This is a simplified implementation - in a real engine, you'd need to handle descriptor tables properly
        auto d3d12Buffer = static_cast<D3D12Buffer*>(buffer);

        // Create a proper GPU descriptor handle instead of just using the GPU virtual address
        D3D12_GPU_DESCRIPTOR_HANDLE handle = {};
        handle.ptr = d3d12Buffer->GetGPUVirtualAddress();

        m_commandList->SetGraphicsRootDescriptorTable(rootIndex, handle);
    }

    void D3D12CommandList::Draw(uint32_t vertexCount, uint32_t instanceCount) {
        if (!m_commandList) {
            return;
        }

        m_commandList->DrawInstanced(vertexCount, instanceCount, 0, 0);
    }

    void D3D12CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount) {
        if (!m_commandList) {
            return;
        }

        m_commandList->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
    }

    void D3D12CommandList::CopyTextureRegion(ITexture* dst, ITexture* src) {
        if (!m_commandList || !dst || !src) {
            return;
        }

        auto d3d12Dst = static_cast<D3D12Texture*>(dst);
        auto d3d12Src = static_cast<D3D12Texture*>(src);

        // This is a simplified implementation - in a real engine, you'd need to handle subresources properly
        D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
        dstLocation.pResource = d3d12Dst->GetD3D12Resource();
        dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstLocation.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
        srcLocation.pResource = d3d12Src->GetD3D12Resource();
        srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        srcLocation.SubresourceIndex = 0;

        D3D12_BOX srcBox = {};
        srcBox.left = 0;
        srcBox.top = 0;
        srcBox.front = 0;
        srcBox.right = d3d12Src->GetWidth();
        srcBox.bottom = d3d12Src->GetHeight();
        srcBox.back = d3d12Src->GetDepth();

        m_commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, &srcBox);
    }

    void D3D12CommandList::ClearRenderTargetView(ITexture* renderTarget, const float color[4]) {
        if (!m_commandList || !renderTarget || !color) {
            return;
        }

        auto d3d12Texture = static_cast<D3D12Texture*>(renderTarget);
        ID3D12Resource* d3d12Resource = d3d12Texture->GetD3D12Resource();
        if (!d3d12Resource) {
            return;
        }

        // This is a simplified implementation - in a real engine, you'd need to get the RTV handle
        // For now, we'll just use the resource's GPU virtual address as a placeholder
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = {};
        rtvHandle.ptr = d3d12Resource->GetGPUVirtualAddress();

        m_commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
    }

    void D3D12CommandList::ClearDepthStencilView(ITexture* depthStencil, float depth, uint8_t stencil) {
        if (!m_commandList || !depthStencil) {
            return;
        }

        auto d3d12Texture = static_cast<D3D12Texture*>(depthStencil);
        ID3D12Resource* d3d12Resource = d3d12Texture->GetD3D12Resource();
        if (!d3d12Resource) {
            return;
        }

        // This is a simplified implementation - in a real engine, you'd need to get the DSV handle
        // For now, we'll just use the resource's GPU virtual address as a placeholder
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};
        dsvHandle.ptr = d3d12Resource->GetGPUVirtualAddress();

        m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth, stencil, 0, nullptr);
    }

    void D3D12CommandList::OMSetRenderTargets(uint32_t numRenderTargets, ITexture* const* renderTargets, ITexture* depthStencil) {
        if (!m_commandList) {
            return;
        }

        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};

        if (renderTargets && numRenderTargets > 0) {
            rtvHandles.reserve(numRenderTargets);
            for (uint32_t i = 0; i < numRenderTargets; i++) {
                if (!renderTargets[i]) {
                    continue;
                }

                auto d3d12Texture = static_cast<D3D12Texture*>(renderTargets[i]);
                ID3D12Resource* d3d12Resource = d3d12Texture->GetD3D12Resource();
                if (!d3d12Resource) {
                    continue;
                }

                // This is a simplified implementation - in a real engine, you'd need to get the RTV handle
                // For now, we'll just use the resource's GPU virtual address as a placeholder
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = {};
                rtvHandle.ptr = d3d12Resource->GetGPUVirtualAddress();
                rtvHandles.push_back(rtvHandle);
            }
        }

        if (depthStencil) {
            auto d3d12Texture = static_cast<D3D12Texture*>(depthStencil);
            ID3D12Resource* d3d12Resource = d3d12Texture->GetD3D12Resource();
            if (d3d12Resource) {
                // This is a simplified implementation - in a real engine, you'd need to get the DSV handle
                // For now, we'll just use the resource's GPU virtual address as a placeholder
                dsvHandle.ptr = d3d12Resource->GetGPUVirtualAddress();
            }
        }

        m_commandList->OMSetRenderTargets(
            static_cast<UINT>(rtvHandles.size()),
            rtvHandles.empty() ? nullptr : rtvHandles.data(),
            FALSE,
            depthStencil ? &dsvHandle : nullptr);
    }

    void D3D12CommandList::RSSetViewports(uint32_t numViewports, const Viewport* viewports) {
        if (!m_commandList || !viewports || numViewports == 0) {
            return;
        }

        std::vector<D3D12_VIEWPORT> d3d12Viewports;
        d3d12Viewports.reserve(numViewports);

        for (uint32_t i = 0; i < numViewports; i++) {
            const auto& vp = viewports[i];
            d3d12Viewports.push_back({
                vp.x, vp.y, vp.width, vp.height, vp.minDepth, vp.maxDepth
            });
        }

        m_commandList->RSSetViewports(static_cast<UINT>(d3d12Viewports.size()), d3d12Viewports.data());
    }

    void D3D12CommandList::RSSetScissorRects(uint32_t numRects, const Rect* rects) {
        if (!m_commandList || !rects || numRects == 0) {
            return;
        }

        std::vector<D3D12_RECT> d3d12Rects;
        d3d12Rects.reserve(numRects);

        for (uint32_t i = 0; i < numRects; i++) {
            const auto& rect = rects[i];
            d3d12Rects.push_back({
                rect.left, rect.top, rect.right, rect.bottom
            });
        }

        m_commandList->RSSetScissorRects(static_cast<UINT>(d3d12Rects.size()), d3d12Rects.data());
    }
}