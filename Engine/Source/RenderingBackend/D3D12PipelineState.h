#pragma once

#include "GraphicsDevice.h"
#include "Resource.h"
#include <d3d12.h>
#include <wrl/client.h>

namespace Reality {
    class D3D12Device;

    class D3D12PipelineState : public PipelineStateBase {
    private:
        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
        D3D12Device* m_device;

        void CreateRootSignature();
        void CreateGraphicsPipelineState();

    public:
        D3D12PipelineState(D3D12Device* device, const PipelineStateDesc& desc);
        ~D3D12PipelineState() = default;

        // Helper methods
        ID3D12PipelineState* GetPipelineState() const { return m_pipelineState.Get(); }
        ID3D12RootSignature* GetRootSignature() const { return m_rootSignature.Get(); }
    };
}
