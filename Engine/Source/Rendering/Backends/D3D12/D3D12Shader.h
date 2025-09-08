#pragma once
#include <Rendering/Resource.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

namespace Reality {
    class D3D12Device;

    class D3D12Shader : public ShaderBase {
    public:
        D3D12Shader(D3D12Device* device, const ShaderDesc& desc);
        ~D3D12Shader();

        bool Compile();

        // IShader interface
        ShaderType GetType() const override { return m_desc.type; }
        const std::string& GetSource() const override { return m_desc.source; }
        const std::string& GetEntryPoint() const override { return m_desc.entryPoint; }
        const std::string& GetTarget() const override { return m_desc.target; }
        void* GetNativeShader() const override { return m_blob.Get(); }

        // D3D12-specific accessors
        ID3DBlob* GetShaderBlob() const { return m_blob.Get(); }

    private:
        D3D12Device* m_device;
        Microsoft::WRL::ComPtr<ID3DBlob> m_blob;
        Microsoft::WRL::ComPtr<ID3DBlob> m_error;
    };
}