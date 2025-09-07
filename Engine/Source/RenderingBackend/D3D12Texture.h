#pragma once

#include "GraphicsDevice.h"
#include "Resource.h"
#include <d3d12.h>
#include <wrl/client.h>

namespace Reality {
    class D3D12Device;

    class D3D12Texture : public TextureBase {
    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
        D3D12_RESOURCE_STATES m_state;
        D3D12Device* m_device;
        bool m_ownsResource;
        
    public:
        // Create from description
        D3D12Texture(D3D12Device* device, const TextureDesc& desc, const void* initialData = nullptr);
        
        // Create from existing resource
        D3D12Texture(D3D12Device* device, ID3D12Resource* resource, D3D12_RESOURCE_STATES state);
        
        ~D3D12Texture();
        
        // ITexture interface
        void UpdateData(const void* data, uint32_t mipLevel, uint32_t arraySlice) override;
        
        // Helper methods
        ID3D12Resource* GetResource() const { return m_resource.Get(); }
        D3D12_RESOURCE_STATES GetState() const { return m_state; }
        void SetState(D3D12_RESOURCE_STATES state) { m_state = state; }
    };
}