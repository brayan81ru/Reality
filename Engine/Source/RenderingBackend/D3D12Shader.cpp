#include "D3D12Shader.h"
#include <cassert>
#include <iostream>

namespace Reality {
    D3D12Shader::D3D12Shader(const ShaderDesc& desc) : ShaderBase(desc) {
        // Determine shader target
        const char* target = desc.target.c_str();
        if (target == nullptr || target[0] == '\0') {
            // Set default target based on shader type
            switch (desc.type) {
                case ShaderType::Vertex:
                    target = "vs_5_0";
                    break;
                case ShaderType::Pixel:
                    target = "ps_5_0";
                    break;
                case ShaderType::Geometry:
                    target = "gs_5_0";
                    break;
                case ShaderType::Hull:
                    target = "hs_5_0";
                    break;
                case ShaderType::Domain:
                    target = "ds_5_0";
                    break;
                case ShaderType::Compute:
                    target = "cs_5_0";
                    break;
                default:
                    assert(false && "Unknown shader type");
                    return;
            }
        }
        
        // Compile shader
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
        HRESULT hr = D3DCompile(
            desc.source.c_str(),
            desc.source.length(),
            nullptr,
            nullptr,
            nullptr,
            desc.entryPoint.c_str(),
            target,
            D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
            0,
            &m_blob,
            &errorBlob
        );
        
        if (FAILED(hr)) {
            if (errorBlob) {
                const char* errorMsg = static_cast<const char*>(errorBlob->GetBufferPointer());
                std::cerr << "Shader compilation error: " << errorMsg << std::endl;
            }
            assert(false && "Failed to compile shader");
            return;
        }
    }
}
