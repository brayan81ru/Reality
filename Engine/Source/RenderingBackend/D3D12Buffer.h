#pragma once

#include "GraphicsDevice.h"
#include "Resource.h"
#include <d3d12.h>
#include <wrl/client.h>

namespace Reality {
    class D3D12Device;

    class D3D12Buffer : public BufferBase {
    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
        D3D12_RESOURCE_STATES m_state;
        D3D12Device* m_device;

    public:
        D3D12Buffer(D3D12Device* device, const BufferDesc& desc, const void* initialData = nullptr);
        ~D3D12Buffer();

        // IBuffer interface
        void* Map() override;
        void Unmap() override;
        void UpdateData(const void* data, size_t size, size_t offset = 0) override;

        // Helper methods
        ID3D12Resource* GetResource() const { return m_resource.Get(); }
        D3D12_RESOURCE_STATES GetState() const { return m_state; }
        void SetState(D3D12_RESOURCE_STATES state) { m_state = state; }
    };
}