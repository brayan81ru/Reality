#pragma once
#include <Rendering/Resource.h>
#include <d3d12.h>
#include <wrl/client.h>

namespace Reality {
    class D3D12Device;

    class D3D12PipelineState : public PipelineStateBase {
    public:
        D3D12PipelineState(D3D12Device* device, const PipelineStateDesc& desc);
        ~D3D12PipelineState();

        bool Initialize();

        // IPipelineState interface
        const PipelineStateDesc& GetDesc() const override { return m_desc; }
        void* GetNativePipelineState() const override { return m_pipelineState.Get(); }

        // D3D12-specific accessors
        ID3D12PipelineState* GetD3D12PipelineState() const { return m_pipelineState.Get(); }
        ID3D12RootSignature* GetRootSignature() const { return m_rootSignature.Get(); }

    private:
        D3D12Device* m_device;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

    };
}