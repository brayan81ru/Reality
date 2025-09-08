#include "D3D12Texture.h"
#include "D3D12Device.h"
#include <cassert>

namespace Reality {
    D3D12Texture::D3D12Texture(D3D12Device* device, const TextureDesc& desc)
        : TextureBase(desc, device), m_device(device) {
        Initialize();
    }

    D3D12Texture::~D3D12Texture() {
    }

    bool D3D12Texture::Initialize() {
        if (!m_device || !m_device->GetD3DDevice()) {
            return false;
        }

        // Determine resource flags
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
        if (m_desc.bindFlags & TextureBindFlags::RenderTarget) {
            flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        }
        if (m_desc.bindFlags & TextureBindFlags::DepthStencil) {
            flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        }
        if (m_desc.bindFlags & TextureBindFlags::UnorderedAccess) {
            flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        // Determine resource state
        if (m_desc.bindFlags & TextureBindFlags::RenderTarget) {
            m_resourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;
        } else if (m_desc.bindFlags & TextureBindFlags::DepthStencil) {
            m_resourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        } else if (m_desc.bindFlags & TextureBindFlags::ShaderResource) {
            m_resourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        } else {
            m_resourceState = D3D12_RESOURCE_STATE_COPY_DEST;
        }

        // Create resource description
        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(m_desc.type);
        resourceDesc.Alignment = 0;
        resourceDesc.Width = m_desc.width;
        resourceDesc.Height = m_desc.height;
        resourceDesc.DepthOrArraySize = static_cast<UINT16>(m_desc.depth * m_desc.arraySize);
        resourceDesc.MipLevels = static_cast<UINT16>(m_desc.mipLevels);
        resourceDesc.Format = static_cast<DXGI_FORMAT>(m_desc.format);
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDesc.Flags = flags;

        // Determine heap properties
        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        // Create the resource
        HRESULT hr = m_device->GetD3DDevice()->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            m_resourceState,
            nullptr,
            IID_PPV_ARGS(&m_resource));

        return SUCCEEDED(hr);
    }

    void D3D12Texture::SetResource(ID3D12Resource* resource) {
        m_resource = resource;
        m_ownsResource = false;
    }

    void D3D12Texture::UpdateData(const void* data, uint32_t mipLevel, uint32_t arraySlice) {
        if (!m_resource || !data) {
            return;
        }

        // This is a simplified implementation - in a real engine, you'd use a staging buffer
        // and command lists to copy the data to the texture

        // For now, we'll just assert that this is not implemented
        assert(false && "Texture update not implemented for D3D12");
    }
}