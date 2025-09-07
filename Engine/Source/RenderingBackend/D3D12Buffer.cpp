#include "D3D12Buffer.h"
#include "D3D12Device.h"
#include <cassert>

namespace Reality {
    D3D12Buffer::D3D12Buffer(D3D12Device* device, const BufferDesc& desc, const void* initialData)
        : BufferBase(desc), m_device(device) {
        ID3D12Device* d3d12Device = device->GetD3DDevice();
        
        // Create resource
        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Alignment = 0;
        resourceDesc.Width = desc.size;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        
        // Determine heap properties and initial state
        D3D12_HEAP_PROPERTIES heapProps = {};
        D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
        
        if (desc.usage == ResourceUsage::Dynamic) {
            heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
            initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
        } else {
            heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
            initialState = D3D12_RESOURCE_STATE_COPY_DEST;
        }
        
        // Create committed resource
        HRESULT hr = d3d12Device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            initialState,
            nullptr,
            IID_PPV_ARGS(&m_resource)
        );
        
        if (FAILED(hr)) {
            // Handle error
            assert(false && "Failed to create buffer");
            return;
        }
        
        m_state = initialState;
        
        // Set initial data if provided
        if (initialData) {
            if (desc.usage == ResourceUsage::Dynamic) {
                // For dynamic resources, we can map and copy directly
                void* mappedData = Map();
                memcpy(mappedData, initialData, desc.size);
                Unmap();
            } else {
                // For default resources, we need to use an upload buffer
                Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
                
                D3D12_HEAP_PROPERTIES uploadHeapProps = {};
                uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
                uploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
                uploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
                uploadHeapProps.CreationNodeMask = 0;
                uploadHeapProps.VisibleNodeMask = 0;
                
                hr = d3d12Device->CreateCommittedResource(
                    &uploadHeapProps,
                    D3D12_HEAP_FLAG_NONE,
                    &resourceDesc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&uploadBuffer)
                );
                
                if (FAILED(hr)) {
                    assert(false && "Failed to create upload buffer");
                    return;
                }
                
                // Map upload buffer and copy data
                void* mappedData;
                hr = uploadBuffer->Map(0, nullptr, &mappedData);
                if (FAILED(hr)) {
                    assert(false && "Failed to map upload buffer");
                    return;
                }
                
                memcpy(mappedData, initialData, desc.size);
                uploadBuffer->Unmap(0, nullptr);
                
                // Copy from upload buffer to default buffer
                ID3D12CommandQueue* commandQueue = device->GetCommandQueue();
                
                Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
                hr = d3d12Device->CreateCommandAllocator(
                    D3D12_COMMAND_LIST_TYPE_DIRECT,
                    IID_PPV_ARGS(&commandAllocator)
                );
                
                if (FAILED(hr)) {
                    assert(false && "Failed to create command allocator");
                    return;
                }
                
                Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
                hr = d3d12Device->CreateCommandList(
                    0,
                    D3D12_COMMAND_LIST_TYPE_DIRECT,
                    commandAllocator.Get(),
                    nullptr,
                    IID_PPV_ARGS(&commandList)
                );
                
                if (FAILED(hr)) {
                    assert(false && "Failed to create command list");
                    return;
                }
                
                // Resource barrier for destination
                D3D12_RESOURCE_BARRIER barrier = {};
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barrier.Transition.pResource = m_resource.Get();
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
                
                commandList->ResourceBarrier(1, &barrier);
                
                // Copy resource
                commandList->CopyResource(m_resource.Get(), uploadBuffer.Get());
                
                // Transition to final state
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
                
                commandList->ResourceBarrier(1, &barrier);
                
                hr = commandList->Close();
                if (FAILED(hr)) {
                    assert(false && "Failed to close command list");
                    return;
                }
                
                // Execute command list
                ID3D12CommandList* commandLists[] = { commandList.Get() };
                commandQueue->ExecuteCommandLists(1, commandLists);
                
                // Wait for completion
                Microsoft::WRL::ComPtr<ID3D12Fence> fence;
                hr = d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
                
                if (FAILED(hr)) {
                    assert(false && "Failed to create fence");
                    return;
                }
                
                HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
                if (!fenceEvent) {
                    assert(false && "Failed to create fence event");
                    return;
                }
                
                UINT64 fenceValue = 1;
                hr = commandQueue->Signal(fence.Get(), fenceValue);
                
                if (FAILED(hr)) {
                    assert(false && "Failed to signal fence");
                    CloseHandle(fenceEvent);
                    return;
                }
                
                if (fence->GetCompletedValue() < fenceValue) {
                    hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
                    
                    if (FAILED(hr)) {
                        assert(false && "Failed to set event on completion");
                        CloseHandle(fenceEvent);
                        return;
                    }
                    
                    WaitForSingleObject(fenceEvent, INFINITE);
                }
                
                CloseHandle(fenceEvent);
                
                // Reset command allocator for future use
                commandAllocator->Reset();
            }
        }
    }
    
    D3D12Buffer::~D3D12Buffer() {
        if (m_mappedData) {
            Unmap();
        }
    }
    
    void* D3D12Buffer::Map() {
        if (!m_mappedData) {
            D3D12_RANGE readRange = { 0, 0 }; // We don't intend to read from this resource on the CPU
            HRESULT hr = m_resource->Map(0, &readRange, &m_mappedData);
            if (FAILED(hr)) {
                return nullptr;
            }
        }
        return m_mappedData;
    }
    
    void D3D12Buffer::Unmap() {
        if (m_mappedData) {
            m_resource->Unmap(0, nullptr);
            m_mappedData = nullptr;
        }
    }
    
    void D3D12Buffer::UpdateData(const void* data, size_t size, size_t offset) {
        void* mappedData = Map();
        if (mappedData) {
            memcpy(static_cast<char*>(mappedData) + offset, data, size);
            Unmap();
        }
    }
}