#include "D3D12Fence.h"
#include "D3D12Device.h"
#include <cassert>

namespace Reality {
    D3D12Fence::D3D12Fence(D3D12Device* device)
        : FenceBase(device), m_device(device) {
        Initialize();
    }

    D3D12Fence::~D3D12Fence() {
        if (m_fenceEvent) {
            CloseHandle(m_fenceEvent);
        }
    }

    bool D3D12Fence::Initialize() {
        if (!m_device || !m_device->GetD3DDevice()) {
            return false;
        }

        // Create fence
        HRESULT hr = m_device->GetD3DDevice()->CreateFence(
            0,
            D3D12_FENCE_FLAG_NONE,
            IID_PPV_ARGS(&m_fence));
        if (FAILED(hr)) {
            return false;
        }

        // Create fence event
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!m_fenceEvent) {
            return false;
        }

        m_lastSignaledValue = 0;
        m_value = 0;

        return true;
    }

    uint64_t D3D12Fence::GetCompletedValue() {
        return GetCompletedValueImpl();
    }

    void D3D12Fence::Signal(uint64_t value) {
        SignalImpl(value);
    }

    void D3D12Fence::Wait(uint64_t value) {
        WaitImpl(value);
    }

    uint64_t D3D12Fence::GetCompletedValueImpl() {
        if (!m_fence) {
            return 0;
        }

        return m_fence->GetCompletedValue();
    }

    void D3D12Fence::SignalImpl(uint64_t value) {
        if (!m_device || !m_device->GetCommandQueue() || !m_fence) {
            return;
        }

        HRESULT hr = m_device->GetCommandQueue()->Signal(m_fence.Get(), value);
        if (SUCCEEDED(hr)) {
            m_lastSignaledValue = value;
        }
    }

    void D3D12Fence::WaitImpl(uint64_t value) {
        if (!m_fence || !m_fenceEvent) {
            return;
        }

        // If the fence has already reached the value, return immediately
        if (m_fence->GetCompletedValue() >= value) {
            return;
        }

        // Set the event to be signaled when the fence reaches the value
        HRESULT hr = m_fence->SetEventOnCompletion(value, m_fenceEvent);
        if (FAILED(hr)) {
            return;
        }

        // Wait for the event
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}