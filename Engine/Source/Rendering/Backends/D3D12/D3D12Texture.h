#pragma once
#include <Rendering/Resource.h>
#include <d3d12.h>
#include <wrl/client.h>

namespace Reality {
    class D3D12Device;

    class D3D12Texture : public TextureBase {
    public:
        D3D12Texture(D3D12Device* device, const TextureDesc& desc);
        ~D3D12Texture();

        bool Initialize();
        void SetResource(ID3D12Resource* resource);

        // ITexture interface
        void UpdateData(const void* data, uint32_t mipLevel, uint32_t arraySlice) override;
        uint32_t GetWidth() const override { return m_desc.width; }
        uint32_t GetHeight() const override { return m_desc.height; }
        uint32_t GetDepth() const override { return m_desc.depth; }
        uint32_t GetMipLevels() const override { return m_desc.mipLevels; }
        uint32_t GetArraySize() const override { return m_desc.arraySize; }
        Format GetFormat() const override { return m_desc.format; }
        ResourceType GetType() const override { return m_desc.type; }
        ResourceUsage GetUsage() const override { return m_desc.usage; }
        void* GetNativeResource() const override { return m_resource.Get(); }

        // D3D12-specific accessors
        ID3D12Resource* GetD3D12Resource() const { return m_resource.Get(); }

    private:
        D3D12Device* m_device;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
        D3D12_RESOURCE_STATES m_resourceState = D3D12_RESOURCE_STATE_COMMON;
        bool m_ownsResource = true;
    };
}