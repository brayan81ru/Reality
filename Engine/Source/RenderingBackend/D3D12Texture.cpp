#include "D3D12Texture.h"
#include "D3D12Device.h"
#include <cassert>

namespace Reality {
    D3D12Texture::D3D12Texture(D3D12Device* device, const TextureDesc& desc, const void* initialData)
        : TextureBase(desc), m_device(device), m_ownsResource(true) {
        ID3D12Device* d3d12Device = device->GetD3DDevice();

        // Create resource description
        D3D12_RESOURCE_DESC resourceDesc = {};

        switch (desc.type) {
            case ResourceType::Texture1D:
                resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
                resourceDesc.DepthOrArraySize = desc.arraySize;
                break;
            case ResourceType::Texture2D:
                resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                resourceDesc.DepthOrArraySize = desc.arraySize;
                break;
            case ResourceType::Texture3D:
                resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                resourceDesc.DepthOrArraySize = desc.depth;
                break;
            case ResourceType::TextureCube:
                resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                resourceDesc.DepthOrArraySize = desc.arraySize * 6; // 6 faces per cube
                break;
            default:
                assert(false && "Unknown resource type");
                return;
        }

        resourceDesc.Alignment = 0;
        resourceDesc.Width = desc.width;
        resourceDesc.Height = desc.height;
        resourceDesc.MipLevels = desc.mipLevels;
        resourceDesc.Format = static_cast<DXGI_FORMAT>(desc.format);
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        // Set flags based on bind flags
        if (desc.bindFlags & TextureBindFlags::RenderTarget) {
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        }
        if (desc.bindFlags & TextureBindFlags::DepthStencil) {
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        }

        // Determine heap properties and initial state
        D3D12_HEAP_PROPERTIES heapProps = {};
        D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COPY_DEST;

        if (desc.usage == ResourceUsage::Dynamic) {
            heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
            initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
        } else {
            heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
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
            assert(false && "Failed to create texture");
            return;
        }

        m_state = initialState;

        // Set initial data if provided
        if (initialData) {
            // Implementation similar to D3D12Buffer
            // This would be more complex for textures due to mip levels and array slices
            // For now, we'll just assert that it's not implemented
            assert(false && "Initial data for textures not implemented yet");
        }
    }

    D3D12Texture::D3D12Texture(D3D12Device* device, ID3D12Resource* resource, D3D12_RESOURCE_STATES state)
        : TextureBase(TextureDesc()), m_device(device), m_ownsResource(false) {
        m_resource = resource;
        m_state = state;

        // Fill in texture description from resource
        D3D12_RESOURCE_DESC desc = resource->GetDesc();
        m_desc.width = static_cast<uint32_t>(desc.Width);
        m_desc.height = desc.Height;
        m_desc.depth = desc.DepthOrArraySize;
        m_desc.mipLevels = desc.MipLevels;
        m_desc.arraySize = desc.DepthOrArraySize;
        m_desc.format = static_cast<Format>(desc.Format);

        // Determine resource type
        switch (desc.Dimension) {
            case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
                m_desc.type = ResourceType::Texture1D;
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
                m_desc.type = ResourceType::Texture2D;
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
                m_desc.type = ResourceType::Texture3D;
                break;
            default:
                m_desc.type = ResourceType::Texture2D; // Default
                break;
        }
    }

    D3D12Texture::~D3D12Texture() {
        // Resource will be released automatically by ComPtr
    }

    void D3D12Texture::UpdateData(const void* data, uint32_t mipLevel, uint32_t arraySlice) {
        // Implementation for updating texture data
        // This would involve creating an upload buffer and copying to the texture
        // For now, we'll just assert that it's not implemented
        assert(false && "UpdateData for textures not implemented yet");
    }
}
