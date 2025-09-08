#include "D3D12Shader.h"
#include "D3D12Device.h"
#include <cassert>

namespace Reality {
    D3D12Shader::D3D12Shader(D3D12Device* device, const ShaderDesc& desc)
        : ShaderBase(desc, device), m_device(device) {
    }

    D3D12Shader::~D3D12Shader() {
    }

    bool D3D12Shader::Compile() {
        if (m_desc.source.empty()) {
            return false;
        }

        // Determine shader target
        std::string target = m_desc.target;
        if (target.empty()) {
            switch (m_desc.type) {
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
                    return false;
            }
        }

        // Compile shader
        HRESULT hr = D3DCompile(
            m_desc.source.c_str(),
            m_desc.source.length(),
            nullptr,
            nullptr,
            nullptr,
            m_desc.entryPoint.c_str(),
            target.c_str(),
            0,
            0,
            &m_blob,
            &m_error);

        if (FAILED(hr)) {
            if (m_error) {
                OutputDebugStringA(reinterpret_cast<const char*>(m_error->GetBufferPointer()));
            }
            return false;
        }

        return true;
    }
}