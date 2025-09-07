#pragma once

#include "GraphicsDevice.h"
#include "Resource.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

namespace Reality {
    class D3D12Shader : public ShaderBase {
    private:
        Microsoft::WRL::ComPtr<ID3DBlob> m_blob;
        
    public:
        D3D12Shader(const ShaderDesc& desc);
        ~D3D12Shader() = default;
        
        // Helper methods
        ID3DBlob* GetBlob() const { return m_blob.Get(); }
        D3D12_SHADER_BYTECODE GetByteCode() const {
            return { m_blob->GetBufferPointer(), m_blob->GetBufferSize() };
        }
    };
}