#include "D3D12Device.h"
#include "D3D12SwapChain.h"
#include "D3D12Buffer.h"
#include "D3D12Texture.h"
#include "D3D12Shader.h"
#include "D3D12PipelineState.h"
#include "D3D12CommandList.h"
#include "D3D12Fence.h"
#include <d3dcompiler.h>
#include <cassert>

namespace Reality {
    D3D12Device::D3D12Device() {
    }

    D3D12Device::~D3D12Device() {
        Shutdown();
    }

    bool D3D12Device::Initialize(const DeviceCreationParams& params) {
        if (m_initialized) {
            return true;
        }

        m_nativeWindow = params.nativeWindow;
        m_width = params.width;
        m_height = params.height;

        // Enable debug layer
#ifdef _DEBUG
        Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
            debugController->EnableDebugLayer();
        }
#endif

        // Create DXGI factory
        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&m_factory));
        if (FAILED(hr)) {
            return false;
        }

        // Create D3D12 device
        if (!CreateD3DDevice(params)) {
            return false;
        }

        // Detect device features
        DetectDeviceFeatures();

        // Create command queue
        CreateCommandQueue();

        // Create descriptor heaps
        CreateDescriptorHeaps();

        m_initialized = true;
        return true;
    }

    void D3D12Device::Shutdown() {
        if (!m_initialized) {
            return;
        }

        WaitForIdle();

        // Release D3D12 objects
        m_device.Reset();
        m_factory.Reset();
        m_commandQueue.Reset();
        m_rtvHeap.Reset();
        m_dsvHeap.Reset();
        m_srvHeap.Reset();
        m_samplerHeap.Reset();

        m_initialized = false;
    }

    bool D3D12Device::CreateD3DDevice(const DeviceCreationParams& params) {
        HRESULT hr;

        // Get hardware adapter
        Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
        if (params.adapterIndex == 0) {
            // Use default adapter
            hr = m_factory->EnumAdapters1(0, &hardwareAdapter);
        } else {
            // Use specified adapter
            hr = m_factory->EnumAdapters1(params.adapterIndex, &hardwareAdapter);
        }

        if (FAILED(hr)) {
            return false;
        }

        // Create D3D12 device
        hr = D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device));

        if (FAILED(hr)) {
            // Try WARP device if hardware failed
            Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
            hr = m_factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
            if (SUCCEEDED(hr)) {
                hr = D3D12CreateDevice(
                    warpAdapter.Get(),
                    D3D_FEATURE_LEVEL_11_0,
                    IID_PPV_ARGS(&m_device));
            }
        }

        return SUCCEEDED(hr);
    }

    void D3D12Device::DetectDeviceFeatures() {
        // Initialize with default values
        m_features = DeviceFeatures();

        if (!m_device) {
            return;
        }

        // Check for ray tracing support
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
        if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)))) {
            m_features.rayTracing = options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
        }

        // Check for mesh shaders support
        D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
        if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7)))) {
            m_features.meshShaders = options7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;
        }

        // Check for variable rate shading support
        D3D12_FEATURE_DATA_D3D12_OPTIONS6 options6 = {};
        if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &options6, sizeof(options6)))) {
            m_features.variableRateShading = options6.VariableShadingRateTier != D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED;
        }

        // Get max texture size
        D3D12_FEATURE_DATA_FORMAT_INFO formatInfo = {};
        formatInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO, &formatInfo, sizeof(formatInfo)))) {
            // This is a simplified check - in reality we'd need to check multiple formats
            m_features.maxTextureSize = 16384; // Default max for D3D12
        }

        // Get max sampler anisotropy
        D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = {};
        formatSupport.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport)))) {
            // This is a simplified check
            m_features.maxSamplerAnisotropy = 16;
        }

        // Get max constant buffer size
        m_features.maxConstantBufferSize = 65536; // D3D12 constant buffer max size

        // Get max vertex attributes
        m_features.maxVertexAttributes = 32; // D3D12 supports up to 32 vertex attributes
    }

    void D3D12Device::CreateCommandQueue() {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        HRESULT hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
        assert(SUCCEEDED(hr));
    }

    void D3D12Device::CreateDescriptorHeaps() {
        // RTV heap
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = 1024; // Arbitrary size
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        HRESULT hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
        assert(SUCCEEDED(hr));
        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // DSV heap
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 128; // Arbitrary size
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));
        assert(SUCCEEDED(hr));
        m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

        // SRV heap
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = 4096; // Arbitrary size
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        hr = m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));
        assert(SUCCEEDED(hr));
        m_srvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        // Sampler heap
        D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
        samplerHeapDesc.NumDescriptors = 256; // Arbitrary size
        samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        hr = m_device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_samplerHeap));
        assert(SUCCEEDED(hr));
        m_samplerDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    }

    ISwapChain* D3D12Device::CreateSwapChain(const SwapChainDesc& desc) {
        auto swapChain = new D3D12SwapChain(this);
        if (!swapChain->Initialize(desc)) {
            delete swapChain;
            return nullptr;
        }
        return swapChain;
    }

    void D3D12Device::DestroySwapChain(ISwapChain* swapChain) {
        auto d3d12SwapChain = static_cast<D3D12SwapChain*>(swapChain);
        delete d3d12SwapChain;
    }

    IBuffer* D3D12Device::CreateBuffer(const BufferDesc& desc, const void* initialData) {
        auto buffer = new D3D12Buffer(this, desc);
        if (initialData) {
            buffer->UpdateData(initialData, desc.size, 0);
        }
        return buffer;
    }

    void D3D12Device::DestroyBuffer(IBuffer* buffer) {
        auto d3d12Buffer = static_cast<D3D12Buffer*>(buffer);
        delete d3d12Buffer;
    }

    ITexture* D3D12Device::CreateTexture(const TextureDesc& desc, const void* initialData) {
        auto texture = new D3D12Texture(this, desc);
        if (initialData) {
            // For simplicity, we'll just update the first mip and first array slice
            texture->UpdateData(initialData, 0, 0);
        }
        return texture;
    }

    void D3D12Device::DestroyTexture(ITexture* texture) {
        auto d3d12Texture = static_cast<D3D12Texture*>(texture);
        delete d3d12Texture;
    }

    IShader* D3D12Device::CreateShader(const ShaderDesc& desc) {
        auto shader = new D3D12Shader(this, desc);
        if (!shader->Compile()) {
            delete shader;
            return nullptr;
        }
        return shader;
    }

    void D3D12Device::DestroyShader(IShader* shader) {
        auto d3d12Shader = static_cast<D3D12Shader*>(shader);
        delete d3d12Shader;
    }

    IPipelineState* D3D12Device::CreatePipelineState(const PipelineStateDesc& desc) {
        auto pipelineState = new D3D12PipelineState(this, desc);
        if (!pipelineState->Initialize()) {
            delete pipelineState;
            return nullptr;
        }
        return pipelineState;
    }

    void D3D12Device::DestroyPipelineState(IPipelineState* pipelineState) {
        auto d3d12PipelineState = static_cast<D3D12PipelineState*>(pipelineState);
        delete d3d12PipelineState;
    }

    ICommandList* D3D12Device::CreateCommandList() {
        auto commandList = new D3D12CommandList(this);
        if (!commandList->Initialize()) {
            delete commandList;
            return nullptr;
        }
        return commandList;
    }

    void D3D12Device::DestroyCommandList(ICommandList* commandList) {
        auto d3d12CommandList = static_cast<D3D12CommandList*>(commandList);
        delete d3d12CommandList;
    }

    void D3D12Device::ExecuteCommandLists(ICommandList* const* commandLists, uint32_t count) {
        if (!m_commandQueue || count == 0) {
            return;
        }

        std::vector<ID3D12CommandList*> d3d12CommandLists;
        d3d12CommandLists.reserve(count);

        for (uint32_t i = 0; i < count; i++) {
            auto d3d12CommandList = static_cast<D3D12CommandList*>(commandLists[i]);
            d3d12CommandLists.push_back(d3d12CommandList->GetD3D12CommandList());
        }

        m_commandQueue->ExecuteCommandLists(static_cast<UINT>(d3d12CommandLists.size()), d3d12CommandLists.data());
    }

    IFence* D3D12Device::CreateFence() {
        auto fence = new D3D12Fence(this);
        if (!fence->Initialize()) {
            delete fence;
            return nullptr;
        }
        return fence;
    }

    void D3D12Device::DestroyFence(IFence* fence) {
        auto d3d12Fence = static_cast<D3D12Fence*>(fence);
        delete d3d12Fence;
    }

    void D3D12Device::WaitForIdle() {
        if (!m_commandQueue) {
            return;
        }

        // Create a temporary fence
        Microsoft::WRL::ComPtr<ID3D12Fence> fence;
        HRESULT hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
        if (FAILED(hr)) {
            return;
        }

        // Signal the fence
        hr = m_commandQueue->Signal(fence.Get(), 1);
        if (FAILED(hr)) {
            return;
        }

        // Wait for the fence
        HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (fenceEvent) {
            fence->SetEventOnCompletion(1, fenceEvent);
            WaitForSingleObject(fenceEvent, INFINITE);
            CloseHandle(fenceEvent);
        }
    }
}
