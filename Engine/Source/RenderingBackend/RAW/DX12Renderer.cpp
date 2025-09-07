#include "DX12Renderer.h"
#include <cstdio>
#include <windows.h>
#include <assert.h>
#include <stdexcept>
#include "d3dx12.h"

namespace Reality {

DX12Renderer::DX12Renderer(HWND hwnd, int width, int height)
    : m_hwnd(hwnd), m_width(width), m_height(height), m_frameIndex(0), m_initialized(false) {
    for (int i = 0; i < FrameCount; i++) {
        m_fenceValues[i] = 0;
    }
    Initialize();
}

DX12Renderer::~DX12Renderer() {
    WaitForPreviousFrame();
    CloseHandle(m_fenceEvent);
}

void DX12Renderer::Initialize() {
    HRESULT hr;

    // Enable debug layer
#ifdef _DEBUG
    Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
    }
#endif

    // Create device
    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        HandleError(hr, "Failed to create DXGI factory");
        return;
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
    hr = factory->EnumAdapters1(0, &hardwareAdapter);
    if (FAILED(hr)) {
        HandleError(hr, "Failed to enumerate adapters");
        return;
    }

    hr = D3D12CreateDevice(
        hardwareAdapter.Get(),
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_device));
    if (FAILED(hr)) {
        HandleError(hr, "Failed to create D3D12 device");
        return;
    }

    // Create command queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
    if (FAILED(hr)) {
        HandleError(hr, "Failed to create command queue");
        return;
    }

    // Create swap chain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
    hr = factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),
        m_hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain);
    if (FAILED(hr)) {
        HandleError(hr, "Failed to create swap chain");
        return;
    }

    hr = factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);
    if (FAILED(hr)) {
        HandleError(hr, "Failed to make window association");
        return;
    }

    hr = swapChain.As(&m_swapChain);
    if (FAILED(hr)) {
        HandleError(hr, "Failed to convert swap chain");
        return;
    }

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap));
    if (FAILED(hr)) {
        HandleError(hr, "Failed to create RTV descriptor heap");
        return;
    }

    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Create render targets
    CreateRenderTargets();

    // Create command allocators
    for (UINT i = 0; i < FrameCount; i++) {
        hr = m_device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&m_commandAllocators[i]));
        if (FAILED(hr)) {
            HandleError(hr, "Failed to create command allocator");
            return;
        }
    }

    // Create command list
    hr = m_device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandAllocators[0].Get(),
        nullptr,
        IID_PPV_ARGS(&m_commandList));
    if (FAILED(hr)) {
        HandleError(hr, "Failed to create command list");
        return;
    }

    // Command lists are created in the recording state, but we'll close it for now
    hr = m_commandList->Close();
    if (FAILED(hr)) {
        HandleError(hr, "Failed to close command list");
        return;
    }

    // Create synchronization objects
    CreateSyncObjects();

    // Create root signature and pipeline state
    CreateRootSignature();
    CreatePipelineState();

    // Create vertex buffer
    CreateVertexBuffer();

    // Set viewport and scissor
    m_viewport = { 0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f };
    m_scissorRect = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };

    m_initialized = true;
}

void DX12Renderer::CreateRenderTargets() {
    HRESULT hr;

    // Create render targets
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < FrameCount; i++) {
        hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
        if (FAILED(hr)) {
            HandleError(hr, "Failed to get swap chain buffer");
            return;
        }
        m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += m_rtvDescriptorSize;
    }
}

void DX12Renderer::CreateSyncObjects() {
    HRESULT hr;

    // Create fence
    hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    if (FAILED(hr)) {
        HandleError(hr, "Failed to create fence");
        return;
    }

    m_fenceValues[0] = 1;
    m_fenceValues[1] = 1;

    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        HandleError(hr, "Failed to create fence event");
        return;
    }
}

void DX12Renderer::CreateRootSignature() {
    HRESULT hr;

    // Create a root signature with a single constant buffer parameter
    D3D12_ROOT_PARAMETER rootParameter = {};
    rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    rootParameter.Constants.ShaderRegister = 0;
    rootParameter.Constants.RegisterSpace = 0;
    rootParameter.Constants.Num32BitValues = 16; // Enough for a 4x4 matrix
    rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = &rootParameter;
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> error;
    hr = D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signature,
        &error);
    if (FAILED(hr)) {
        if (error) {
            OutputDebugStringA((char*)error->GetBufferPointer());
        }
        HandleError(hr, "Failed to serialize root signature");
        return;
    }

    hr = m_device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(&m_rootSignature));
    if (FAILED(hr)) {
        HandleError(hr, "Failed to create root signature");
        return;
    }
}

void DX12Renderer::CreateVertexBuffer() {
    HRESULT hr;

    // Define the vertex structure
    struct Vertex {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
    };

    // Define the triangle geometry
    Vertex triangleVertices[] = {
        { { 0.0f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };

    const UINT vertexBufferSize = sizeof(triangleVertices);

    // Create vertex buffer resource
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = vertexBufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    hr = m_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer));
    if (FAILED(hr)) {
        HandleError(hr, "Failed to create vertex buffer");
        return;
    }

    // Map and copy vertex data
    UINT8* pVertexDataBegin;
    D3D12_RANGE readRange = { 0, 0 };
    hr = m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
    if (FAILED(hr)) {
        HandleError(hr, "Failed to map vertex buffer");
        return;
    }
    memcpy(pVertexDataBegin, triangleVertices, vertexBufferSize);
    m_vertexBuffer->Unmap(0, nullptr);

    // Create vertex buffer view
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
    m_vertexBufferView.SizeInBytes = vertexBufferSize;
}

void DX12Renderer::CreatePipelineState() {
    HRESULT hr;

    // Create shaders
    const char* vertexShaderSource = R"(
        struct VS_IN {
            float3 pos : POSITION;
            float4 col : COLOR;
        };

        struct VS_OUT {
            float4 pos : SV_POSITION;
            float4 col : COLOR;
        };

        VS_OUT main(VS_IN input) {
            VS_OUT output;
            output.pos = float4(input.pos, 1.0f);
            output.col = input.col;
            return output;
        }
    )";

    const char* pixelShaderSource = R"(
        struct PS_IN {
            float4 pos : SV_POSITION;
            float4 col : COLOR;
        };

        float4 main(PS_IN input) : SV_TARGET {
            return input.col;
        }
    )";

    Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
    Microsoft::WRL::ComPtr<ID3DBlob> error;

    hr = D3DCompile(
        vertexShaderSource,
        strlen(vertexShaderSource),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "vs_5_0",
        0,
        0,
        &vertexShader,
        &error);
    if (FAILED(hr)) {
        if (error) {
            OutputDebugStringA((char*)error->GetBufferPointer());
        }
        HandleError(hr, "Failed to compile vertex shader");
        return;
    }

    hr = D3DCompile(
        pixelShaderSource,
        strlen(pixelShaderSource),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        0,
        0,
        &pixelShader,
        &error);
    if (FAILED(hr)) {
        if (error) {
            OutputDebugStringA((char*)error->GetBufferPointer());
        }
        HandleError(hr, "Failed to compile pixel shader");
        return;
    }

    // Define the vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Create the graphics pipeline state
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
    if (FAILED(hr)) {
        HandleError(hr, "Failed to create pipeline state");
        return;
    }
}

void DX12Renderer::Render() {
    if (!m_initialized) {
        return;
    }

    // Wait for the previous frame to finish
    WaitForPreviousFrame();

    // Reset command allocator and command list
    HRESULT hr = m_commandAllocators[m_frameIndex]->Reset();
    if (FAILED(hr)) {
        HandleError(hr, "Failed to reset command allocator");
        return;
    }

    hr = m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get());
    if (FAILED(hr)) {
        HandleError(hr, "Failed to reset command list");
        return;
    }

    // Set necessary state
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_commandList->ResourceBarrier(1, &barrier);

    // Set render target
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += m_frameIndex * m_rtvDescriptorSize;
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Record commands
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->DrawInstanced(3, 1, 0, 0);

    // Indicate that the back buffer will now be used for presentation
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    m_commandList->ResourceBarrier(1, &barrier);

    // Close command list
    hr = m_commandList->Close();
    if (FAILED(hr)) {
        HandleError(hr, "Failed to close command list");
        return;
    }

    // Execute command list
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present
    hr = m_swapChain->Present(1, 0);
    if (FAILED(hr)) {
        HandleError(hr, "Failed to present swap chain");
        return;
    }

    // Update the frame index
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void DX12Renderer::WaitForPreviousFrame() {
    if (!m_fence || !m_commandQueue) {
        return;
    }

    // Signal the fence
    const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
    HRESULT hr = m_commandQueue->Signal(m_fence.Get(), currentFenceValue);
    if (FAILED(hr)) {
        HandleError(hr, "Failed to signal fence");
        return;
    }

    // Wait until the previous frame is finished
    if (m_fence->GetCompletedValue() < currentFenceValue) {
        hr = m_fence->SetEventOnCompletion(currentFenceValue, m_fenceEvent);
        if (FAILED(hr)) {
            HandleError(hr, "Failed to set fence event");
            return;
        }
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    // Update the fence value for the next frame
    m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}

void DX12Renderer::Resize(int width, int height) {
    if (!m_initialized || width == 0 || height == 0) {
        return;
    }

    // Wait for all previous GPU work to complete
    WaitForPreviousFrame();

    // Release previous resources
    for (UINT i = 0; i < FrameCount; i++) {
        m_renderTargets[i].Reset();
    }

    // Resize the swap chain
    HRESULT hr = m_swapChain->ResizeBuffers(
        FrameCount,
        width, height,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        0);
    if (FAILED(hr)) {
        HandleError(hr, "Failed to resize swap chain buffers");
        return;
    }

    m_width = width;
    m_height = height;
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Update the viewport and scissor rect
    m_viewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
    m_scissorRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };

    // Recreate render targets
    CreateRenderTargets();
}

void DX12Renderer::HandleError(HRESULT hr, const std::string& message) {
    char errorMsg[512];
    DWORD length = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        hr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        errorMsg,
        sizeof(errorMsg),
        nullptr);

    char buffer[1024];
    if (length > 0) {
        sprintf_s(buffer, "%s\nError: 0x%08X\n%s", message.c_str(), hr, errorMsg);
    } else {
        sprintf_s(buffer, "%s\nError: 0x%08X", message.c_str(), hr);
    }

    OutputDebugStringA(buffer);
    OutputDebugStringA("\n");

    // Also check for device removed reasons
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        if (m_device) {
            HRESULT deviceRemovedReason = m_device->GetDeviceRemovedReason();
            sprintf_s(buffer, "Device removed reason: 0x%08X\n", deviceRemovedReason);
            OutputDebugStringA(buffer);
        }
    }
}

} // namespace Reality