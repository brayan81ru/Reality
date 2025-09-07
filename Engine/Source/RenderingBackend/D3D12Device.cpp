#include "D3D12Device.h"
#include "D3D12Buffer.h"
#include "D3D12Texture.h"
#include "D3D12Shader.h"
#include "D3D12PipelineState.h"
#include "D3D12CommandList.h"
#include "D3D12Fence.h"
#include <cassert>
#include <iostream>

namespace Reality {
    D3D12Device::D3D12Device() = default;
    
    D3D12Device::~D3D12Device() {
        Shutdown();
    }
    
    void D3D12Device::Initialize(void* nativeWindow) {
        CreateDeviceResources();
        CreateSwapChainResources(nativeWindow);
        CreateDescriptorHeaps();
        CreateRenderTargetViews();
        CreateDepthStencilBuffers();
    }
    
    void D3D12Device::Shutdown() {
        WaitForIdle();
        
        m_backBuffers.clear();
        m_depthBuffers.clear();
        m_rtvHandles.clear();
        m_dsvHandles.clear();
        
        m_rtvHeap.Reset();
        m_dsvHeap.Reset();
        m_commandAllocator.Reset();
        m_swapChain.Reset();
        m_commandQueue.Reset();
        m_dxgiFactory.Reset();
        m_device.Reset();
    }
    
    void D3D12Device::Resize(uint32_t width, uint32_t height) {
        if (m_width == width && m_height == height) {
            return;
        }
        
        m_width = width;
        m_height = height;
        
        // Flush GPU before resize
        WaitForIdle();
        
        // Release resources
        for (auto& buffer : m_backBuffers) {
            buffer.Reset();
        }
        for (auto& buffer : m_depthBuffers) {
            buffer.Reset();
        }
        m_rtvHandles.clear();
        m_dsvHandles.clear();
        
        // Resize swap chain
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        m_swapChain->GetDesc(&swapChainDesc);
        HRESULT hr = m_swapChain->ResizeBuffers(
            swapChainDesc.BufferCount,
            width,
            height,
            swapChainDesc.BufferDesc.Format,
            m_tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0
        );
        
        if (FAILED(hr)) {
            std::cerr << "Failed to resize swap chain: " << hr << std::endl;
            return;
        }
        
        // Recreate resources
        CreateRenderTargetViews();
        CreateDepthStencilBuffers();
    }
    
    void D3D12Device::CreateDeviceResources() {
        // Enable debug layer in debug builds
        Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
            debugController->EnableDebugLayer();
        }
        
        // Create DXGI factory
        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory));
        if (FAILED(hr)) {
            std::cerr << "Failed to create DXGI factory: " << hr << std::endl;
            return;
        }
        
        // Check tearing support
        CheckTearingSupport();
        
        // Find hardware adapter
        Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
        for (UINT adapterIndex = 0; 
             DXGI_ERROR_NOT_FOUND != m_dxgiFactory->EnumAdapters1(adapterIndex, &hardwareAdapter); 
             ++adapterIndex) {
            DXGI_ADAPTER_DESC1 desc;
            hardwareAdapter->GetDesc1(&desc);
            
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                continue; // Skip software adapters
            }
            
            // Check if adapter supports D3D12
            if (SUCCEEDED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_0, 
                                           IID_PPV_ARGS(&m_device)))) {
                break;
                                           }
             }
        
        if (!m_device) {
            // Fallback to WARP device
            Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
            hr = m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
            if (FAILED(hr)) {
                std::cerr << "Failed to get WARP adapter: " << hr << std::endl;
                return;
            }
            
            hr = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, 
                                   IID_PPV_ARGS(&m_device));
            if (FAILED(hr)) {
                std::cerr << "Failed to create D3D12 device: " << hr << std::endl;
                return;
            }
        }
        
        // Create command queue
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 0;
        
        hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
        if (FAILED(hr)) {
            std::cerr << "Failed to create command queue: " << hr << std::endl;
            return;
        }
        
        // Create command allocator
        hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                              IID_PPV_ARGS(&m_commandAllocator));
        if (FAILED(hr)) {
            std::cerr << "Failed to create command allocator: " << hr << std::endl;
            return;
        }
        
        // Get descriptor sizes
        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    }
    
    void D3D12Device::CreateSwapChainResources(void* nativeWindow) {
        HWND hWnd = static_cast<HWND>(nativeWindow);
        
        // Get window size
        RECT rect;
        GetClientRect(hWnd, &rect);
        m_width = rect.right - rect.left;
        m_height = rect.bottom - rect.top;
        
        // Create swap chain
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = m_width;
        swapChainDesc.Height = m_height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2; // Double buffering
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesc.Flags = m_tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
        
        Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
        HRESULT hr = m_dxgiFactory->CreateSwapChainForHwnd(
            m_commandQueue.Get(),
            hWnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain
        );
        
        if (FAILED(hr)) {
            std::cerr << "Failed to create swap chain: " << hr << std::endl;
            return;
        }
        
        // Disable Alt+Enter fullscreen transition
        m_dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
        
        // Get swap chain 4 interface
        hr = swapChain.As(&m_swapChain);
        if (FAILED(hr)) {
            std::cerr << "Failed to get swap chain 4: " << hr << std::endl;
            return;
        }
    }
    
    void D3D12Device::CreateDescriptorHeaps() {
        // Create RTV descriptor heap
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.NumDescriptors = 2; // Double buffering
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvHeapDesc.NodeMask = 0;
        
        HRESULT hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
        if (FAILED(hr)) {
            std::cerr << "Failed to create RTV descriptor heap: " << hr << std::endl;
            return;
        }
        
        // Create DSV descriptor heap
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.NumDescriptors = 2; // Double buffering
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        dsvHeapDesc.NodeMask = 0;
        
        hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));
        if (FAILED(hr)) {
            std::cerr << "Failed to create DSV descriptor heap: " << hr << std::endl;
            return;
        }
    }
    
    void D3D12Device::CreateRenderTargetViews() {
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

        for (UINT i = 0; i < 2; i++) {
            Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;
            HRESULT hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
            if (FAILED(hr)) {
                std::cerr << "Failed to get back buffer " << i << ": " << hr << std::endl;
                return;
            }

            m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
            m_backBuffers.push_back(backBuffer);
            m_rtvHandles.push_back(rtvHandle);

            rtvHandle.ptr += m_rtvDescriptorSize;
        }
    }
    
    void D3D12Device::CreateDepthStencilBuffers() {
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
        
        for (UINT i = 0; i < 2; i++) {
            // Create depth stencil texture
            D3D12_RESOURCE_DESC depthDesc = {};
            depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            depthDesc.Alignment = 0;
            depthDesc.Width = m_width;
            depthDesc.Height = m_height;
            depthDesc.DepthOrArraySize = 1;
            depthDesc.MipLevels = 1;
            depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
            depthDesc.SampleDesc.Count = 1;
            depthDesc.SampleDesc.Quality = 0;
            depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            
            D3D12_HEAP_PROPERTIES heapProps = {};
            heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
            heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heapProps.CreationNodeMask = 0;
            heapProps.VisibleNodeMask = 0;
            
            Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer;
            HRESULT hr = m_device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &depthDesc,
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                nullptr,
                IID_PPV_ARGS(&depthBuffer)
            );
            
            if (FAILED(hr)) {
                std::cerr << "Failed to create depth buffer " << i << ": " << hr << std::endl;
                return;
            }
            
            // Create depth stencil view
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
            dsvDesc.Texture2D.MipSlice = 0;

            m_device->CreateDepthStencilView(depthBuffer.Get(), &dsvDesc, dsvHandle);
            m_depthBuffers.push_back(depthBuffer);
            m_dsvHandles.push_back(dsvHandle);

            dsvHandle.ptr += m_dsvDescriptorSize;
        }
    }
    
    void D3D12Device::CheckTearingSupport() {
        Microsoft::WRL::ComPtr<IDXGIFactory5> factory5;
        HRESULT hr = m_dxgiFactory.As(&factory5);
        if (FAILED(hr)) {
            m_tearingSupported = false;
            return;
        }
        
        BOOL allowTearing = FALSE;
        hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, 
                                          &allowTearing, sizeof(allowTearing));
        m_tearingSupported = (SUCCEEDED(hr) && allowTearing);
    }
    
    IBuffer* D3D12Device::CreateBuffer(const BufferDesc& desc, const void* initialData) {
        return new D3D12Buffer(this, desc, initialData);
    }
    
    ITexture* D3D12Device::CreateTexture(const TextureDesc& desc, const void* initialData) {
        return new D3D12Texture(this, desc, initialData);
    }
    
    IShader* D3D12Device::CreateShader(const ShaderDesc& desc) {
        return new D3D12Shader(desc);
    }
    
    IPipelineState* D3D12Device::CreatePipelineState(const PipelineStateDesc& desc) {
        return new D3D12PipelineState(this, desc);
    }
    
    ICommandList* D3D12Device::CreateCommandList() {
        return new D3D12CommandList(this);
    }
    
    void D3D12Device::ExecuteCommandLists(ICommandList* const* commandLists, uint32_t count) {
        std::vector<ID3D12CommandList*> d3d12CommandLists(count);
        for (uint32_t i = 0; i < count; i++) {
            D3D12CommandList* d3d12CommandList = static_cast<D3D12CommandList*>(commandLists[i]);
            d3d12CommandLists[i] = d3d12CommandList->GetCommandList();
        }
        
        m_commandQueue->ExecuteCommandLists(count, d3d12CommandLists.data());
    }
    
    IFence* D3D12Device::CreateFence() {
        return new D3D12Fence(this);
    }
    
    void D3D12Device::WaitForIdle() {
        Microsoft::WRL::ComPtr<ID3D12Fence> fence;
        HRESULT hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
        if (FAILED(hr)) {
            std::cerr << "Failed to create fence: " << hr << std::endl;
            return;
        }
        
        HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!fenceEvent) {
            std::cerr << "Failed to create fence event" << std::endl;
            return;
        }
        
        UINT64 fenceValue = 1;
        hr = m_commandQueue->Signal(fence.Get(), fenceValue);
        if (FAILED(hr)) {
            std::cerr << "Failed to signal fence: " << hr << std::endl;
            CloseHandle(fenceEvent);
            return;
        }
        
        if (fence->GetCompletedValue() < fenceValue) {
            hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
            if (FAILED(hr)) {
                std::cerr << "Failed to set event on completion: " << hr << std::endl;
                CloseHandle(fenceEvent);
                return;
            }
            
            WaitForSingleObject(fenceEvent, INFINITE);
        }
        
        CloseHandle(fenceEvent);
    }

    void D3D12Device::Present() {
        HRESULT hr = m_swapChain->Present(1, m_tearingSupported ? DXGI_PRESENT_ALLOW_TEARING : 0);
        if (FAILED(hr)) {
            std::cerr << "Failed to present: " << hr << std::endl;
            return;
        }
        
        m_currentBackBuffer = (m_currentBackBuffer + 1) % 2;
    }
    
    uint32_t D3D12Device::GetBackBufferIndex() const {
        return m_currentBackBuffer;
    }
    
    ITexture* D3D12Device::GetBackBuffer(uint32_t index) {
        if (index >= m_backBuffers.size()) {
            return nullptr;
        }

        // Use the new constructor that includes the RTV handle
        return new D3D12Texture(this, m_backBuffers[index].Get(), m_rtvHandles[index],
                                D3D12_RESOURCE_STATE_RENDER_TARGET);
    }
}