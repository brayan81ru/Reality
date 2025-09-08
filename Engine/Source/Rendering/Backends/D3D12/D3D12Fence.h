#pragma once
#include <Rendering/Resource.h>
#include <d3d12.h>
#include <wrl/client.h>

namespace Reality {
    class D3D12Device;

    class D3D12Fence : public FenceBase {
    public:
        D3D12Fence(D3D12Device* device);
        ~D3D12Fence();

        bool Initialize();

        // IFence interface
        uint64_t GetCompletedValue() override;
        void Signal(uint64_t value) override;
        void Wait(uint64_t value) override;
        void* GetNativeFence() const override { return m_fence.Get(); }

        // D3D12-specific accessors
        ID3D12Fence* GetD3D12Fence() const { return m_fence.Get(); }

    protected:
        uint64_t GetCompletedValueImpl() override;
        void SignalImpl(uint64_t value) override;
        void WaitImpl(uint64_t value) override;

    private:
        D3D12Device* m_device;
        Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
        HANDLE m_fenceEvent = nullptr;
        uint64_t m_lastSignaledValue = 0;
    };
}