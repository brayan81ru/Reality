#include "D3D12PipelineState.h"
#include "D3D12Device.h"
#include "D3D12Shader.h"
#include <cassert>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <iostream>

// Helper structures for DirectX 12
struct CD3DX12_ROOT_PARAMETER1 {
    D3D12_ROOT_PARAMETER1 parameter;
    void InitAsConstantBufferView(UINT shaderRegister, UINT registerSpace = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) {
        parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        parameter.ShaderVisibility = visibility;
        parameter.Descriptor.ShaderRegister = shaderRegister;
        parameter.Descriptor.RegisterSpace = registerSpace;
        parameter.Descriptor.Flags = flags;
    }
};

struct CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC {
    D3D12_VERSIONED_ROOT_SIGNATURE_DESC desc;
    void Init_1_1(UINT numParameters, const D3D12_ROOT_PARAMETER1* pParameters, UINT numStaticSamplers = 0, const D3D12_STATIC_SAMPLER_DESC* pStaticSamplers = nullptr, D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE) {
        desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
        desc.Desc_1_1.NumParameters = numParameters;
        desc.Desc_1_1.pParameters = pParameters;
        desc.Desc_1_1.NumStaticSamplers = numStaticSamplers;
        desc.Desc_1_1.pStaticSamplers = pStaticSamplers;
        desc.Desc_1_1.Flags = flags;
    }
};

struct CD3DX12_BLEND_DESC : public D3D12_BLEND_DESC {
    CD3DX12_BLEND_DESC() {
        AlphaToCoverageEnable = FALSE;
        IndependentBlendEnable = FALSE;
        const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = {
            FALSE,FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL
        };
        for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
            RenderTarget[i] = defaultRenderTargetBlendDesc;
    }
};

struct CD3DX12_RASTERIZER_DESC : public D3D12_RASTERIZER_DESC {
    CD3DX12_RASTERIZER_DESC() {
        FillMode = D3D12_FILL_MODE_SOLID;
        CullMode = D3D12_CULL_MODE_BACK;
        FrontCounterClockwise = FALSE;
        DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        DepthClipEnable = TRUE;
        MultisampleEnable = FALSE;
        AntialiasedLineEnable = FALSE;
        ForcedSampleCount = 0;
        ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    }
};

struct CD3DX12_DEPTH_STENCIL_DESC : public D3D12_DEPTH_STENCIL_DESC {
    CD3DX12_DEPTH_STENCIL_DESC() {
        DepthEnable = TRUE;
        DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        StencilEnable = FALSE;
        StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp = {
            D3D12_STENCIL_OP_KEEP,
            D3D12_STENCIL_OP_KEEP,
            D3D12_STENCIL_OP_KEEP,
            D3D12_COMPARISON_FUNC_ALWAYS
        };
        FrontFace = defaultStencilOp;
        BackFace = defaultStencilOp;
    }
};

namespace Reality {
    D3D12PipelineState::D3D12PipelineState(D3D12Device* device, const PipelineStateDesc& desc)
        : PipelineStateBase(desc), m_device(device) {
        CreateRootSignature();
        CreateGraphicsPipelineState();
    }

    void D3D12PipelineState::CreateRootSignature() {
        ID3D12Device* d3d12Device = m_device->GetD3DDevice();

        // Create a simple root signature with a single CBV
        CD3DX12_ROOT_PARAMETER1 rootParameters[1];
        rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(
            _countof(rootParameters),
            &rootParameters[0].parameter,  // Pass pointer to the D3D12_ROOT_PARAMETER1
            0,
            nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        );

        Microsoft::WRL::ComPtr<ID3DBlob> signature;
        Microsoft::WRL::ComPtr<ID3DBlob> error;

        HRESULT hr = D3D12SerializeVersionedRootSignature(
            &rootSignatureDesc.desc,
            &signature,
            &error
        );

        if (FAILED(hr)) {
            std::cerr << "Failed to serialize root signature: " << hr << std::endl;
            if (error) {
                std::cerr << "Error: " << (const char*)error->GetBufferPointer() << std::endl;
            }
            return;
        }

        hr = d3d12Device->CreateRootSignature(
            0,
            signature->GetBufferPointer(),
            signature->GetBufferSize(),
            IID_PPV_ARGS(&m_rootSignature)
        );

        if (FAILED(hr)) {
            std::cerr << "Failed to create root signature: " << hr << std::endl;
            return;
        }
    }

    void D3D12PipelineState::CreateGraphicsPipelineState() {
        ID3D12Device* d3d12Device = m_device->GetD3DDevice();

        // Create shaders from shader descriptions
        D3D12Shader* vertexShader = new D3D12Shader(m_desc.vertexShader);
        D3D12Shader* pixelShader = new D3D12Shader(m_desc.pixelShader);

        // Check if shaders were created successfully
        if (!vertexShader || !vertexShader->GetBlob()) {
            std::cerr << "Failed to create vertex shader" << std::endl;
            delete vertexShader;
            delete pixelShader;
            return;
        }

        if (!pixelShader || !pixelShader->GetBlob()) {
            std::cerr << "Failed to create pixel shader" << std::endl;
            delete vertexShader;
            delete pixelShader;
            return;
        }

        // Create input layout
        std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
        if (m_desc.inputElements && m_desc.numInputElements > 0) {
            inputElements.resize(m_desc.numInputElements);
            for (uint32_t i = 0; i < m_desc.numInputElements; i++) {
                const InputElementDesc& element = m_desc.inputElements[i];
                inputElements[i] = {
                    element.semanticName,
                    element.semanticIndex,
                    static_cast<DXGI_FORMAT>(element.format),
                    element.inputSlot,
                    element.alignedByteOffset,
                    static_cast<D3D12_INPUT_CLASSIFICATION>(element.inputSlotClass),
                    element.instanceDataStepRate
                };
            }
        }

        // Create pipeline state description
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS = vertexShader->GetByteCode();
        psoDesc.PS = pixelShader->GetByteCode();

        // Set blend state
        psoDesc.BlendState = CD3DX12_BLEND_DESC();
        psoDesc.SampleMask = UINT_MAX;

        // Set rasterizer state
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC();

        // Set depth stencil state
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC();

        // Set input layout
        psoDesc.InputLayout.pInputElementDescs = inputElements.data();
        psoDesc.InputLayout.NumElements = static_cast<UINT>(inputElements.size());

        // Set primitive topology
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

        // Set render targets
        psoDesc.NumRenderTargets = m_desc.numRenderTargets;
        for (uint32_t i = 0; i < m_desc.numRenderTargets; i++) {
            psoDesc.RTVFormats[i] = static_cast<DXGI_FORMAT>(m_desc.renderTargetFormats[i]);
        }

        // Set depth stencil format
        psoDesc.DSVFormat = static_cast<DXGI_FORMAT>(m_desc.depthStencilFormat);

        // Set sample description
        psoDesc.SampleDesc.Count = m_desc.sampleCount;
        psoDesc.SampleDesc.Quality = m_desc.sampleQuality;

        // Set node mask
        psoDesc.NodeMask = 0;

        // Set cached PSO
        psoDesc.CachedPSO.pCachedBlob = nullptr;
        psoDesc.CachedPSO.CachedBlobSizeInBytes = 0;

        // Set flags
        psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

        // Validate the pipeline state description
        if (!psoDesc.pRootSignature) {
            std::cerr << "Root signature is null" << std::endl;
            delete vertexShader;
            delete pixelShader;
            return;
        }

        if (!psoDesc.VS.pShaderBytecode || psoDesc.VS.BytecodeLength == 0) {
            std::cerr << "Vertex shader is invalid" << std::endl;
            delete vertexShader;
            delete pixelShader;
            return;
        }

        if (!psoDesc.PS.pShaderBytecode || psoDesc.PS.BytecodeLength == 0) {
            std::cerr << "Pixel shader is invalid" << std::endl;
            delete vertexShader;
            delete pixelShader;
            return;
        }

        if (psoDesc.NumRenderTargets > 0 && psoDesc.RTVFormats[0] == DXGI_FORMAT_UNKNOWN) {
            std::cerr << "Render target format is invalid" << std::endl;
            delete vertexShader;
            delete pixelShader;
            return;
        }

        // Create pipeline state
        HRESULT hr = d3d12Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
        if (FAILED(hr)) {
            std::cerr << "Failed to create pipeline state: " << hr << std::endl;

            // Print more detailed information about the failure
            std::cerr << "Pipeline State Description:" << std::endl;
            std::cerr << "  Root Signature: " << (psoDesc.pRootSignature ? "Valid" : "Null") << std::endl;
            std::cerr << "  Vertex Shader: " << (psoDesc.VS.pShaderBytecode ? "Valid" : "Null") <<
                         ", Size: " << psoDesc.VS.BytecodeLength << std::endl;
            std::cerr << "  Pixel Shader: " << (psoDesc.PS.pShaderBytecode ? "Valid" : "Null") <<
                         ", Size: " << psoDesc.PS.BytecodeLength << std::endl;
            std::cerr << "  Num Render Targets: " << psoDesc.NumRenderTargets << std::endl;
            std::cerr << "  RTV Format: " << psoDesc.RTVFormats[0] << std::endl;
            std::cerr << "  DSV Format: " << psoDesc.DSVFormat << std::endl;
            std::cerr << "  Sample Count: " << psoDesc.SampleDesc.Count << std::endl;
            std::cerr << "  Input Layout Count: " << psoDesc.InputLayout.NumElements << std::endl;

            /*
            // Get more detailed error information if available
            Microsoft::WRL::ComPtr<ID3D12DeviceRemovedExtendedData> pDred;
            if (SUCCEEDED(d3d12Device->QueryInterface(IID_PPV_ARGS(&pDred)))) {
                D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT breadcrumbs;
                if (SUCCEEDED(pDred->GetAutoBreadcrumbsOutput(&breadcrumbs))) {
                    std::cerr << "DRED Breadcrumbs:" << std::endl;
                    for (UINT i = 0; i < breadcrumbs.NumBreadcrumbContexts; i++) {
                        const auto& context = breadcrumbs.pBreadcrumbContexts[i];
                        std::cerr << "Context " << i << ": " << context.pBreadcrumbEntries->pCommandListDebugNameA << std::endl;
                    }
                }
            }
            */

            delete vertexShader;
            delete pixelShader;
            return;
        }

        // Clean up shaders
        delete vertexShader;
        delete pixelShader;
    }
}