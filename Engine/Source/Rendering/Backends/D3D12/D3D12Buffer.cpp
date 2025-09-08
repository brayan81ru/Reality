#include "D3D12Buffer.h"
#include "D3D12Device.h"
#include <cassert>

namespace Reality {
    D3D12Buffer::D3D12Buffer(D3D12Device* device, const BufferDesc& desc)
        : BufferBase(desc, device), m_device(device) {
        Initialize();
    }

    D3D12Buffer::~D3D12Buffer() {
        if (m_mappedData) {
            Unmap();
        }
    }

    bool D3D12Buffer::Initialize() {
        if (!m_device || !m_device->GetD3DDevice()) {
            return false;
        }

        // Determine heap properties
        D3D12_HEAP_PROPERTIES heapProps = {};
        if (m_desc.usage == ResourceUsage::Dynamic) {
            heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        } else if (m_desc.cpuAccessFlags & CPUAccessFlags::Read) {
            heapProps.Type = D3D12_HEAP_TYPE_READBACK;
        } else {
            heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        }
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        // Determine resource state
        if (m_desc.usage == ResourceUsage::Dynamic) {
            m_resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
        } else {
            m_resourceState = D3D12_RESOURCE_STATE_COMMON;
        }

        // Determine resource flags
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
        if (m_desc.bindFlags & BufferBindFlags::UnorderedAccess) {
            flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        // Create resource description
        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Alignment = 0;
        resourceDesc.Width = m_desc.size;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDesc.Flags = flags;

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

    void* D3D12Buffer::Map() {
        if (!m_resource || m_mappedData) {
            return m_mappedData;
        }

        D3D12_RANGE readRange = { 0, 0 };
        HRESULT hr = m_resource->Map(0, &readRange, &m_mappedData);
        if (FAILED(hr)) {
            m_mappedData = nullptr;
        }

        return m_mappedData;
    }

    void D3D12Buffer::Unmap() {
        if (!m_resource || !m_mappedData) {
            return;
        }

        m_resource->Unmap(0, nullptr);
        m_mappedData = nullptr;
    }

    void D3D12Buffer::UpdateData(const void* data, size_t size, size_t offset) {
        if (!m_resource || !data || size == 0) {
            return;
        }

        if (m_desc.usage == ResourceUsage::Dynamic) {
            // For dynamic resources, map and copy
            void* mappedData = Map();
            if (mappedData) {
                memcpy(static_cast<uint8_t*>(mappedData) + offset, data, size);
                Unmap();
            }
        } else {
            // For default resources, we need to use an upload buffer
            // This is a simplified implementation - in a real engine, you'd use a staging buffer
            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
            heapProps.CreationNodeMask = 1;
            heapProps.VisibleNodeMask = 1;

            D3D12_RESOURCE_DESC resourceDesc = {};
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            resourceDesc.Alignment = 0;
            resourceDesc.Width = size;
            resourceDesc.Height = 1;
            resourceDesc.DepthOrArraySize = 1;
            resourceDesc.MipLevels = 1;
            resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
            resourceDesc.SampleDesc.Count = 1;
            resourceDesc.SampleDesc.Quality = 0;
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
            HRESULT hr = m_device->GetD3DDevice()->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&uploadBuffer));

            if (SUCCEEDED(hr)) {
                // Map and copy to upload buffer
                void* mappedData;
                D3D12_RANGE readRange = { 0, 0 };
                hr = uploadBuffer->Map(0, &readRange, &mappedData);
                if (SUCCEEDED(hr)) {
                    memcpy(mappedData, data, size);
                    uploadBuffer->Unmap(0, nullptr);

                    // Copy from upload buffer to resource
                    // This would require a command list - simplified for now
                    // In a real implementation, you'd use a command list to copy the data
                }
            }
        }
    }
}