#include "D3D12Fence.h"
#include "D3D12Device.h"
#include <cassert>

namespace Reality {
    D3D12Fence::D3D12Fence(D3D12Device* device) : m_device(device) {
        ID3D12Device* d3d12Device = device->GetD3DDevice();

        // Create fence
        HRESULT hr = d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
        if (FAILED(hr)) {
            assert(false && "Failed to create fence");
            return;
        }

        // Create fence event
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!m_fenceEvent) {
            assert(false && "Failed to create fence event");
            return;
        }
    }

    D3D12Fence::~D3D12Fence() {
        if (m_fenceEvent) {
            CloseHandle(m_fenceEvent);
        }
    }

    uint64_t D3D12Fence::GetCompletedValueImpl() {
        return m_fence->GetCompletedValue();
    }

    void D3D12Fence::SignalImpl(uint64_t value) {
        ID3D12CommandQueue* commandQueue = m_device->GetCommandQueue();
        HRESULT hr = commandQueue->Signal(m_fence.Get(), value);
        if (FAILED(hr)) {
            assert(false && "Failed to signal fence");
            return;
        }
    }

    void D3D12Fence::WaitImpl(uint64_t value) {
        if (m_fence->GetCompletedValue() < value) {
            HRESULT hr = m_fence->SetEventOnCompletion(value, m_fenceEvent);
            if (FAILED(hr)) {
                assert(false && "Failed to set event on completion");
                return;
            }

            WaitForSingleObject(m_fenceEvent, INFINITE);
        }
    }
}
