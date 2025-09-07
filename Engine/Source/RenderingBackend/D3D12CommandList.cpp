#include "D3D12CommandList.h"
#include "D3D12Device.h"
#include "D3D12Texture.h"
#include "D3D12Buffer.h"
#include "D3D12PipelineState.h"
#include <cassert>
#include <iostream>

namespace Reality {
    D3D12CommandList::D3D12CommandList(D3D12Device* device) : m_device(device) {
        ID3D12Device* d3d12Device = device->GetD3DDevice();

        // Create command allocator
        HRESULT hr = d3d12Device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&m_commandAllocator)
        );

        if (FAILED(hr)) {
            assert(false && "Failed to create command allocator");
            return;
        }

        // Create command list
        hr = d3d12Device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_commandAllocator.Get(),
            nullptr,
            IID_PPV_ARGS(&m_commandList)
        );

        if (FAILED(hr)) {
            assert(false && "Failed to create command list");
            return;
        }

        // Start in closed state
        m_commandList->Close();
        m_isClosed = true;
    }

    void D3D12CommandList::ResourceBarrier(ITexture* resource, ResourceState before, ResourceState after) {
        assert(!m_isClosed && "Command list is closed");

        D3D12Texture* d3d12Texture = static_cast<D3D12Texture*>(resource);

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = d3d12Texture->GetResource();
        barrier.Transition.StateBefore = static_cast<D3D12_RESOURCE_STATES>(before);
        barrier.Transition.StateAfter = static_cast<D3D12_RESOURCE_STATES>(after);
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        m_barriers.push_back(barrier);

        // If we have accumulated enough barriers, flush them
        if (m_barriers.size() >= 16) {
            m_commandList->ResourceBarrier(static_cast<UINT>(m_barriers.size()), m_barriers.data());
            m_barriers.clear();
        }
    }

    void D3D12CommandList::SetPipelineState(IPipelineState* pipeline) {
        assert(!m_isClosed && "Command list is closed");

        D3D12PipelineState* d3d12Pipeline = static_cast<D3D12PipelineState*>(pipeline);
        m_commandList->SetPipelineState(d3d12Pipeline->GetPipelineState());
        m_commandList->SetGraphicsRootSignature(d3d12Pipeline->GetRootSignature());
    }

    void D3D12CommandList::SetVertexBuffers(IBuffer* const* buffers, uint32_t startSlot, uint32_t numBuffers) {
        assert(!m_isClosed && "Command list is closed");

        std::vector<D3D12_VERTEX_BUFFER_VIEW> views(numBuffers);
        for (uint32_t i = 0; i < numBuffers; i++) {
            D3D12Buffer* d3d12Buffer = static_cast<D3D12Buffer*>(buffers[i]);
            views[i].BufferLocation = d3d12Buffer->GetResource()->GetGPUVirtualAddress();
            views[i].SizeInBytes = d3d12Buffer->GetSize();
            views[i].StrideInBytes = d3d12Buffer->GetStride();
        }

        m_commandList->IASetVertexBuffers(startSlot, numBuffers, views.data());
    }

    void D3D12CommandList::SetIndexBuffer(IBuffer* buffer) {
        assert(!m_isClosed && "Command list is closed");

        D3D12Buffer* d3d12Buffer = static_cast<D3D12Buffer*>(buffer);

        D3D12_INDEX_BUFFER_VIEW view = {};
        view.BufferLocation = d3d12Buffer->GetResource()->GetGPUVirtualAddress();
        view.SizeInBytes = d3d12Buffer->GetSize();
        view.Format = DXGI_FORMAT_R32_UINT; // Assuming 32-bit indices

        m_commandList->IASetIndexBuffer(&view);
    }

    void D3D12CommandList::SetGraphicsRootConstantBufferView(uint32_t rootIndex, IBuffer* buffer) {
        assert(!m_isClosed && "Command list is closed");

        D3D12Buffer* d3d12Buffer = static_cast<D3D12Buffer*>(buffer);
        m_commandList->SetGraphicsRootConstantBufferView(rootIndex, d3d12Buffer->GetResource()->GetGPUVirtualAddress());
    }

    void D3D12CommandList::SetGraphicsRootDescriptorTable(uint32_t rootIndex, IBuffer* buffer) {
        assert(!m_isClosed && "Command list is closed");
        // Implementation for descriptor tables would go here
        // For now, we'll just assert that it's not implemented
        assert(false && "SetGraphicsRootDescriptorTable not implemented yet");
    }

    void D3D12CommandList::Draw(uint32_t vertexCount, uint32_t instanceCount) {
        assert(!m_isClosed && "Command list is closed");

        // Flush any pending barriers
        if (!m_barriers.empty()) {
            m_commandList->ResourceBarrier(static_cast<UINT>(m_barriers.size()), m_barriers.data());
            m_barriers.clear();
        }

        m_commandList->DrawInstanced(vertexCount, instanceCount, 0, 0);
    }

    void D3D12CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount) {
        assert(!m_isClosed && "Command list is closed");

        // Flush any pending barriers
        if (!m_barriers.empty()) {
            m_commandList->ResourceBarrier(static_cast<UINT>(m_barriers.size()), m_barriers.data());
            m_barriers.clear();
        }

        m_commandList->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
    }

    void D3D12CommandList::CopyTextureRegion(ITexture* dst, ITexture* src) {
        assert(!m_isClosed && "Command list is closed");

        D3D12Texture* d3d12Dst = static_cast<D3D12Texture*>(dst);
        D3D12Texture* d3d12Src = static_cast<D3D12Texture*>(src);

        // For now, we'll do a simple resource copy
        // A more complete implementation would handle subresources and regions
        m_commandList->CopyResource(d3d12Dst->GetResource(), d3d12Src->GetResource());
    }

    void D3D12CommandList::ClearRenderTargetView(ITexture* renderTarget, const float color[4]) {
        assert(!m_isClosed && "Command list is closed");
        D3D12Texture* d3d12Texture = static_cast<D3D12Texture*>(renderTarget);

        // Get RTV handle from the texture
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = d3d12Texture->GetRTV();

        m_commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
    }

    void D3D12CommandList::ClearDepthStencilView(ITexture* depthStencil, float depth, uint8_t stencil) {
        assert(!m_isClosed && "Command list is closed");
        D3D12Texture* d3d12Texture = static_cast<D3D12Texture*>(depthStencil);

        // Get DSV handle from the texture
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = d3d12Texture->GetDSV();

        m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth, stencil, 0, nullptr);
    }

    void D3D12CommandList::OMSetRenderTargets(uint32_t numRenderTargets, ITexture* const* renderTargets, ITexture* depthStencil) {
        assert(!m_isClosed && "Command list is closed");

        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles(numRenderTargets);
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};

        // Get RTV handles
        for (uint32_t i = 0; i < numRenderTargets; i++) {
            D3D12Texture* d3d12Texture = static_cast<D3D12Texture*>(renderTargets[i]);
            rtvHandles[i] = d3d12Texture->GetRTV();
        }

        // Get DSV handle
        if (depthStencil) {
            D3D12Texture* d3d12Texture = static_cast<D3D12Texture*>(depthStencil);
            dsvHandle = d3d12Texture->GetDSV();
        }

        m_commandList->OMSetRenderTargets(numRenderTargets, rtvHandles.data(), FALSE, depthStencil ? &dsvHandle : nullptr);
    }

    
    void D3D12CommandList::RSSetViewports(uint32_t numViewports, const Viewport* viewports) {
        assert(!m_isClosed && "Command list is closed");

        std::vector<D3D12_VIEWPORT> d3d12Viewports(numViewports);
        for (uint32_t i = 0; i < numViewports; i++) {
            d3d12Viewports[i] = {
                viewports[i].x,
                viewports[i].y,
                viewports[i].width,
                viewports[i].height,
                viewports[i].minDepth,
                viewports[i].maxDepth
            };
        }

        m_commandList->RSSetViewports(numViewports, d3d12Viewports.data());
    }

    void D3D12CommandList::RSSetScissorRects(uint32_t numRects, const Rect* rects) {
        assert(!m_isClosed && "Command list is closed");

        std::vector<D3D12_RECT> d3d12Rects(numRects);
        for (uint32_t i = 0; i < numRects; i++) {
            d3d12Rects[i] = {
                rects[i].left,
                rects[i].top,
                rects[i].right,
                rects[i].bottom
            };
        }

        m_commandList->RSSetScissorRects(numRects, d3d12Rects.data());
    }

    void D3D12CommandList::ResetImpl() {
        // Reset command allocator
        HRESULT hr = m_commandAllocator->Reset();
        if (FAILED(hr)) {
            std::cerr << "Failed to reset command allocator: " << hr << std::endl;
            return;
        }

        // Reset command list
        hr = m_commandList->Reset(m_commandAllocator.Get(), nullptr);
        if (FAILED(hr)) {
            std::cerr << "Failed to reset command list: " << hr << std::endl;
            return;
        }

        // Clear barriers
        m_barriers.clear();
    }

    void D3D12CommandList::CloseImpl() {
        // Flush any pending barriers
        if (!m_barriers.empty()) {
            m_commandList->ResourceBarrier(static_cast<UINT>(m_barriers.size()), m_barriers.data());
            m_barriers.clear();
        }

        // Close command list
        HRESULT hr = m_commandList->Close();
        if (FAILED(hr)) {
            assert(false && "Failed to close command list");
            return;
        }
    }
}
