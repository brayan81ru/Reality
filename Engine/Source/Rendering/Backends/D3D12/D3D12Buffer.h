#pragma once
#include <Rendering/Resource.h>
#include <d3d12.h>
#include <wrl/client.h>

namespace Reality {
    class D3D12Device;

    class D3D12Buffer : public BufferBase {
    public:
        D3D12Buffer(D3D12Device* device, const BufferDesc& desc);
        ~D3D12Buffer();

        bool Initialize();

        // IBuffer interface
        void* Map() override;
        void Unmap() override;
        void UpdateData(const void* data, size_t size, size_t offset = 0) override;
        uint32_t GetSize() const override { return m_desc.size; }
        uint32_t GetStride() const override { return m_desc.stride; }
        ResourceUsage GetUsage() const override { return m_desc.usage; }
        void* GetNativeResource() const override { return m_resource.Get(); }

        // D3D12-specific accessors
        ID3D12Resource* GetD3D12Resource() const { return m_resource.Get(); }
        D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
            return m_resource ? m_resource->GetGPUVirtualAddress() : 0;
        }

    private:
        D3D12Device* m_device;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
        D3D12_RESOURCE_STATES m_resourceState = D3D12_RESOURCE_STATE_COMMON;
        void* m_mappedData = nullptr;
    };
}