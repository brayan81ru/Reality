#pragma once
#include <Rendering/GraphicsDevice.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace Reality {
    class D3D12Device;

    class D3D12SwapChain : public ISwapChain {
    public:
        D3D12SwapChain(D3D12Device* device);
        ~D3D12SwapChain();

        bool Initialize(const SwapChainDesc& desc);

        // ISwapChain interface
        void Present(uint32_t SyncInterval = 1) override;
        void Resize(uint32_t width, uint32_t height) override;
        void SetFullscreen(bool fullscreen) override;
        void SetVSync(bool vsync) override;

        uint32_t GetWidth() const override { return m_width; }
        uint32_t GetHeight() const override { return m_height; }
        uint32_t GetBackBufferCount() const override { return m_bufferCount; }
        ITexture* GetBackBuffer(uint32_t index) override;
        uint32_t GetCurrentBackBufferIndex() const override { return m_currentBackBuffer; }

        // D3D12-specific accessors
        IDXGISwapChain4* GetDXGISwapChain() const { return m_swapChain.Get(); }

    private:
        void CreateRenderTargets();
        void ReleaseRenderTargets();

        D3D12Device* m_device;
        Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;

        // Back buffers
        static const uint32_t MaxBackBuffers = 8;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_backBuffers[MaxBackBuffers];
        D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandles[MaxBackBuffers];

        // Swap chain parameters
        void* m_nativeWindow = nullptr;
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        uint32_t m_bufferCount = 2;
        DXGI_FORMAT m_format = DXGI_FORMAT_R8G8B8A8_UNORM;
        bool m_vsync = true;
        bool m_fullscreen = false;
        uint32_t m_currentBackBuffer = 0;

        // Descriptor heap for RTVs
        uint32_t m_rtvDescriptorSize = 0;
    };
}