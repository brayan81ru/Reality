#pragma once

#include "GraphicsDevice.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <vector>

namespace Reality {
    class D3D12Device : public IGraphicsDevice {
    private:
        Microsoft::WRL::ComPtr<ID3D12Device> m_device;
        Microsoft::WRL::ComPtr<IDXGIFactory6> m_dxgiFactory;
        Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
        
        std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_backBuffers;
        std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_depthBuffers;
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_rtvHandles;
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_dsvHandles;
        
        uint32_t m_rtvDescriptorSize = 0;
        uint32_t m_dsvDescriptorSize = 0;
        uint32_t m_currentBackBuffer = 0;
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        bool m_tearingSupported = false;
        
        void CreateDeviceResources();
        void CreateSwapChainResources(void* nativeWindow);
        void CreateDescriptorHeaps();
        void CreateRenderTargetViews();
        void CreateDepthStencilBuffers();
        void CheckTearingSupport();
        
    public:
        D3D12Device();
        ~D3D12Device();
        
        // IGraphicsDevice interface
        void Initialize(void* nativeWindow) override;
        void Shutdown() override;
        void Resize(uint32_t width, uint32_t height) override;
        
        IBuffer* CreateBuffer(const BufferDesc& desc, const void* initialData = nullptr) override;
        ITexture* CreateTexture(const TextureDesc& desc, const void* initialData = nullptr) override;
        IShader* CreateShader(const ShaderDesc& desc) override;
        IPipelineState* CreatePipelineState(const PipelineStateDesc& desc) override;
        
        ICommandList* CreateCommandList() override;
        void ExecuteCommandLists(ICommandList* const* commandLists, uint32_t count) override;
        
        IFence* CreateFence() override;
        void WaitForIdle() override;
        
        void Present() override;
        uint32_t GetBackBufferIndex() const override;
        ITexture* GetBackBuffer(uint32_t index) override;
        
        // Helper methods
        ID3D12Device* GetD3DDevice() const { return m_device.Get(); }
        ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue.Get(); }
        ID3D12DescriptorHeap* GetRTVHeap() const { return m_rtvHeap.Get(); }
        ID3D12DescriptorHeap* GetDSVHeap() const { return m_dsvHeap.Get(); }
    };
}