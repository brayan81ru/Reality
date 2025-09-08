#include "D3D12PipelineState.h"
#include "D3D12Device.h"
#include "D3D12Shader.h"
#include <cassert>

namespace Reality {
    D3D12PipelineState::D3D12PipelineState(D3D12Device* device, const PipelineStateDesc& desc)
        : PipelineStateBase(desc, device), m_device(device) {
    }

    D3D12PipelineState::~D3D12PipelineState() {
    }

    bool D3D12PipelineState::Initialize() {
        if (!m_device || !m_device->GetD3DDevice()) {
            return false;
        }

        // Create root signature (simplified for now)
        D3D12_ROOT_PARAMETER rootParameter = {};
        rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        rootParameter.Constants.ShaderRegister = 0;
        rootParameter.Constants.RegisterSpace = 0;
        rootParameter.Constants.Num32BitValues = 16; // Enough for a 4x4 matrix
        rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        rootSignatureDesc.NumParameters = 1;
        rootSignatureDesc.pParameters = &rootParameter;
        rootSignatureDesc.NumStaticSamplers = 0;
        rootSignatureDesc.pStaticSamplers = nullptr;
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        Microsoft::WRL::ComPtr<ID3DBlob> signature;
        Microsoft::WRL::ComPtr<ID3DBlob> error;
        HRESULT hr = D3D12SerializeRootSignature(
            &rootSignatureDesc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            &signature,
            &error);
        if (FAILED(hr)) {
            if (error) {
                OutputDebugStringA(reinterpret_cast<const char*>(error->GetBufferPointer()));
            }
            return false;
        }

        hr = m_device->GetD3DDevice()->CreateRootSignature(
            0,
            signature->GetBufferPointer(),
            signature->GetBufferSize(),
            IID_PPV_ARGS(&m_rootSignature));
        if (FAILED(hr)) {
            return false;
        }

        // Create input layout
        std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
        if (m_desc.inputElements && m_desc.numInputElements > 0) {
            inputElements.reserve(m_desc.numInputElements);
            for (uint32_t i = 0; i < m_desc.numInputElements; i++) {
                const auto& elem = m_desc.inputElements[i];
                inputElements.push_back({
                    elem.semanticName,
                    elem.semanticIndex,
                    static_cast<DXGI_FORMAT>(elem.format),
                    elem.inputSlot,
                    elem.alignedByteOffset,
                    static_cast<D3D12_INPUT_CLASSIFICATION>(elem.inputSlotClass),
                    elem.instanceDataStepRate
                });
            }
        }

        // Get shader blobs - Fixed: Properly cast from IShader* to D3D12Shader*
        Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
        Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
        Microsoft::WRL::ComPtr<ID3DBlob> computeShader;

        // Create temporary shader objects to get the compiled blobs
        ShaderPtr vsShader, psShader, csShader;

        if (m_desc.vertexShader.source.length() > 0) {
            vsShader = ShaderPtr(m_device->CreateShader(m_desc.vertexShader));
            if (vsShader) {
                auto d3dVs = static_cast<D3D12Shader*>(vsShader.get());
                vertexShader = d3dVs->GetShaderBlob();
            }
        }

        if (m_desc.pixelShader.source.length() > 0) {
            psShader = ShaderPtr(m_device->CreateShader(m_desc.pixelShader));
            if (psShader) {
                auto d3dPs = static_cast<D3D12Shader*>(psShader.get());
                pixelShader = d3dPs->GetShaderBlob();
            }
        }

        if (m_desc.computeShader.source.length() > 0) {
            csShader = ShaderPtr(m_device->CreateShader(m_desc.computeShader));
            if (csShader) {
                auto d3dCs = static_cast<D3D12Shader*>(csShader.get());
                computeShader = d3dCs->GetShaderBlob();
            }
        }

        // Determine if this is a graphics or compute pipeline
        if (computeShader) {
            // Create compute pipeline state
            D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.pRootSignature = m_rootSignature.Get();
            psoDesc.CS = { computeShader->GetBufferPointer(), computeShader->GetBufferSize() };

            hr = m_device->GetD3DDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
        } else {
            // Create graphics pipeline state
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.InputLayout = { inputElements.data(), static_cast<UINT>(inputElements.size()) };
            psoDesc.pRootSignature = m_rootSignature.Get();

            if (vertexShader) {
                psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
            }

            if (pixelShader) {
                psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
            }

            // Set rasterizer state
            psoDesc.RasterizerState.FillMode = static_cast<D3D12_FILL_MODE>(m_desc.rasterizerState.fillMode);
            psoDesc.RasterizerState.CullMode = static_cast<D3D12_CULL_MODE>(m_desc.rasterizerState.cullMode);
            psoDesc.RasterizerState.FrontCounterClockwise = m_desc.rasterizerState.frontCounterClockwise;
            psoDesc.RasterizerState.DepthBias = m_desc.rasterizerState.depthBias;
            psoDesc.RasterizerState.DepthBiasClamp = m_desc.rasterizerState.depthBiasClamp;
            psoDesc.RasterizerState.SlopeScaledDepthBias = m_desc.rasterizerState.slopeScaledDepthBias;
            psoDesc.RasterizerState.DepthClipEnable = m_desc.rasterizerState.depthClipEnable;
            psoDesc.RasterizerState.MultisampleEnable = m_desc.rasterizerState.multisampleEnable;
            psoDesc.RasterizerState.AntialiasedLineEnable = m_desc.rasterizerState.antialiasedLineEnable;
            psoDesc.RasterizerState.ForcedSampleCount = m_desc.rasterizerState.forcedSampleCount;
            psoDesc.RasterizerState.ConservativeRaster = m_desc.rasterizerState.conservativeRaster ?
                D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

            // Set blend state
            psoDesc.BlendState.AlphaToCoverageEnable = m_desc.blendState.alphaToCoverageEnable;
            psoDesc.BlendState.IndependentBlendEnable = m_desc.blendState.independentBlendEnable;
            for (uint32_t i = 0; i < 8; i++) {
                psoDesc.BlendState.RenderTarget[i].BlendEnable = m_desc.blendState.renderTarget[i].blendEnable;
                psoDesc.BlendState.RenderTarget[i].LogicOpEnable = FALSE;
                psoDesc.BlendState.RenderTarget[i].SrcBlend = static_cast<D3D12_BLEND>(m_desc.blendState.renderTarget[i].srcBlend);
                psoDesc.BlendState.RenderTarget[i].DestBlend = static_cast<D3D12_BLEND>(m_desc.blendState.renderTarget[i].destBlend);
                psoDesc.BlendState.RenderTarget[i].BlendOp = static_cast<D3D12_BLEND_OP>(m_desc.blendState.renderTarget[i].blendOp);
                psoDesc.BlendState.RenderTarget[i].SrcBlendAlpha = static_cast<D3D12_BLEND>(m_desc.blendState.renderTarget[i].srcBlendAlpha);
                psoDesc.BlendState.RenderTarget[i].DestBlendAlpha = static_cast<D3D12_BLEND>(m_desc.blendState.renderTarget[i].destBlendAlpha);
                psoDesc.BlendState.RenderTarget[i].BlendOpAlpha = static_cast<D3D12_BLEND_OP>(m_desc.blendState.renderTarget[i].blendOpAlpha);
                psoDesc.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
                psoDesc.BlendState.RenderTarget[i].RenderTargetWriteMask = m_desc.blendState.renderTarget[i].renderTargetWriteMask;
            }

            // Set depth stencil state
            psoDesc.DepthStencilState.DepthEnable = m_desc.depthStencilState.depthEnable;
            psoDesc.DepthStencilState.DepthWriteMask = m_desc.depthStencilState.depthWriteMask ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
            psoDesc.DepthStencilState.DepthFunc = static_cast<D3D12_COMPARISON_FUNC>(m_desc.depthStencilState.depthFunc);
            psoDesc.DepthStencilState.StencilEnable = m_desc.depthStencilState.stencilEnable;
            psoDesc.DepthStencilState.StencilReadMask = m_desc.depthStencilState.stencilReadMask;
            psoDesc.DepthStencilState.StencilWriteMask = m_desc.depthStencilState.stencilWriteMask;
            psoDesc.DepthStencilState.FrontFace.StencilFailOp = static_cast<D3D12_STENCIL_OP>(m_desc.depthStencilState.frontFace.stencilFailOp);
            psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = static_cast<D3D12_STENCIL_OP>(m_desc.depthStencilState.frontFace.stencilDepthFailOp);
            psoDesc.DepthStencilState.FrontFace.StencilPassOp = static_cast<D3D12_STENCIL_OP>(m_desc.depthStencilState.frontFace.stencilPassOp);
            psoDesc.DepthStencilState.FrontFace.StencilFunc = static_cast<D3D12_COMPARISON_FUNC>(m_desc.depthStencilState.frontFace.stencilFunc);
            psoDesc.DepthStencilState.BackFace.StencilFailOp = static_cast<D3D12_STENCIL_OP>(m_desc.depthStencilState.backFace.stencilFailOp);
            psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = static_cast<D3D12_STENCIL_OP>(m_desc.depthStencilState.backFace.stencilDepthFailOp);
            psoDesc.DepthStencilState.BackFace.StencilPassOp = static_cast<D3D12_STENCIL_OP>(m_desc.depthStencilState.backFace.stencilPassOp);
            psoDesc.DepthStencilState.BackFace.StencilFunc = static_cast<D3D12_COMPARISON_FUNC>(m_desc.depthStencilState.backFace.stencilFunc);

            // Set other properties
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            psoDesc.NumRenderTargets = m_desc.numRenderTargets;
            for (uint32_t i = 0; i < 8; i++) {
                psoDesc.RTVFormats[i] = static_cast<DXGI_FORMAT>(m_desc.renderTargetFormats[i]);
            }
            psoDesc.DSVFormat = static_cast<DXGI_FORMAT>(m_desc.depthStencilFormat);
            psoDesc.SampleDesc.Count = m_desc.sampleCount;
            psoDesc.SampleDesc.Quality = m_desc.sampleQuality;

            hr = m_device->GetD3DDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
        }

        return SUCCEEDED(hr);
    }
}