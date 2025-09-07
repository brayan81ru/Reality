#pragma once

#include "GraphicsDevice.h"
#include "Resource.h"
#include <d3d12.h>
#include <wrl/client.h>

namespace Reality {
    class D3D12Device;

    class D3D12Fence : public FenceBase {
    private:
        Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
        D3D12Device* m_device;
        HANDLE m_fenceEvent;
        
    public:
        D3D12Fence(D3D12Device* device);
        ~D3D12Fence();
        
        // Helper methods
        ID3D12Fence* GetFence() const { return m_fence.Get(); }
        
    protected:
        uint64_t GetCompletedValueImpl() override;
        void SignalImpl(uint64_t value) override;
        void WaitImpl(uint64_t value) override;
    };
}