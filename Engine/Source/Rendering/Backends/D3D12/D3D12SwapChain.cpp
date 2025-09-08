#include "D3D12SwapChain.h"
#include "D3D12Device.h"
#include "D3D12Texture.h"
#include <cassert>

namespace Reality {
    D3D12SwapChain::D3D12SwapChain(D3D12Device* device)
        : m_device(device) {
    }

    D3D12SwapChain::~D3D12SwapChain() {
        ReleaseRenderTargets();
    }

    bool D3D12SwapChain::Initialize(const SwapChainDesc& desc) {
        if (!m_device || !m_device->GetD3DDevice()) {
            return false;
        }

        m_nativeWindow = desc.nativeWindow;
        m_width = desc.width;
        m_height = desc.height;
        m_bufferCount = desc.bufferCount;
        m_format = static_cast<DXGI_FORMAT>(desc.format);
        m_vsync = desc.vsync;
        m_fullscreen = desc.fullscreen;

        // Clamp buffer count
        if (m_bufferCount < 1) m_bufferCount = 1;
        if (m_bufferCount > MaxBackBuffers) m_bufferCount = MaxBackBuffers;

        // Get RTV descriptor size
        m_rtvDescriptorSize = m_device->GetD3DDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Create swap chain
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = m_bufferCount;
        swapChainDesc.Width = m_width;
        swapChainDesc.Height = m_height;
        swapChainDesc.Format = m_format;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;

        Microsoft::WRL::ComPtr<IDXGIFactory6> factory;
        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
        if (FAILED(hr)) {
            return false;
        }

        Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
        hr = factory->CreateSwapChainForHwnd(
            m_device->GetCommandQueue(),
            static_cast<HWND>(m_nativeWindow),
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain);
        if (FAILED(hr)) {
            return false;
        }

        // Disable Alt+Enter
        hr = factory->MakeWindowAssociation(static_cast<HWND>(m_nativeWindow), DXGI_MWA_NO_ALT_ENTER);
        if (FAILED(hr)) {
            return false;
        }

        // Get swap chain4
        hr = swapChain.As(&m_swapChain);
        if (FAILED(hr)) {
            return false;
        }

        m_currentBackBuffer = m_swapChain->GetCurrentBackBufferIndex();

        // Create render targets
        CreateRenderTargets();

        return true;
    }

    void D3D12SwapChain::CreateRenderTargets() {
        if (!m_device || !m_device->GetD3DDevice()) {
            return;
        }

        // Create RTV descriptor heap
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.NumDescriptors = m_bufferCount;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvHeapDesc.NodeMask = 0;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
        HRESULT hr = m_device->GetD3DDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
        if (FAILED(hr)) {
            return;
        }

        // Store the heap for later use
        m_rtvHeap = rtvHeap;

        // Get the RTV handle
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

        for (UINT i = 0; i < m_bufferCount; i++) {
            // Get back buffer
            hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i]));
            assert(SUCCEEDED(hr));

            // Create RTV
            m_device->GetD3DDevice()->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, rtvHandle);
            m_rtvHandles[i] = rtvHandle;

            // Move to next descriptor
            rtvHandle.ptr += m_rtvDescriptorSize;
        }
    }

    void D3D12SwapChain::ReleaseRenderTargets() {
        for (UINT i = 0; i < m_bufferCount; i++) {
            m_backBuffers[i].Reset();
        }
    }

    void D3D12SwapChain::Present(uint32_t SyncInterval) {
        if (!m_swapChain) {
            return;
        }

        UINT syncInterval = m_vsync ? SyncInterval : 0;
        m_swapChain->Present(syncInterval, 0);
        m_currentBackBuffer = m_swapChain->GetCurrentBackBufferIndex();
    }

    void D3D12SwapChain::Resize(uint32_t width, uint32_t height) {
        if (!m_swapChain || width == 0 || height == 0) {
            return;
        }

        // Wait for GPU to finish
        m_device->WaitForIdle();

        // Release render targets
        ReleaseRenderTargets();

        // Resize swap chain
        HRESULT hr = m_swapChain->ResizeBuffers(
            m_bufferCount,
            width,
            height,
            m_format,
            0);
        if (FAILED(hr)) {
            return;
        }

        m_width = width;
        m_height = height;
        m_currentBackBuffer = m_swapChain->GetCurrentBackBufferIndex();

        // Recreate render targets
        CreateRenderTargets();
    }

    void D3D12SwapChain::SetFullscreen(bool fullscreen) {
        if (!m_swapChain || m_fullscreen == fullscreen) {
            return;
        }

        HRESULT hr = m_swapChain->SetFullscreenState(fullscreen, nullptr);
        if (SUCCEEDED(hr)) {
            m_fullscreen = fullscreen;
        }
    }

    void D3D12SwapChain::SetVSync(bool vsync) {
        m_vsync = vsync;
    }

    ITexture* D3D12SwapChain::GetBackBuffer(uint32_t index) {
        if (index >= m_bufferCount || !m_backBuffers[index]) {
            return nullptr;
        }

        // Create a texture wrapper for the back buffer
        TextureDesc desc;
        desc.type = ResourceType::Texture2D;
        desc.width = m_width;
        desc.height = m_height;
        desc.depth = 1;
        desc.mipLevels = 1;
        desc.arraySize = 1;
        desc.format = static_cast<Format>(m_format);
        desc.usage = ResourceUsage::Default;
        desc.bindFlags = TextureBindFlags::RenderTarget;

        auto texture = new D3D12Texture(m_device, desc);
        texture->SetResource(m_backBuffers[index].Get());
        return texture;
    }
}