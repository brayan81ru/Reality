#pragma once
#include <Rendering/GraphicsDevice.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <memory>

namespace Reality {
    class D3D12SwapChain;
    class D3D12Buffer;
    class D3D12Texture;
    class D3D12Shader;
    class D3D12PipelineState;
    class D3D12CommandList;
    class D3D12Fence;

    class D3D12Device : public IGraphicsDevice {
    public:
        D3D12Device();
        ~D3D12Device();

        // IGraphicsDevice interface
        bool Initialize(const DeviceCreationParams& params) override;
        void Shutdown() override;

        ISwapChain* CreateSwapChain(const SwapChainDesc& desc) override;
        void DestroySwapChain(ISwapChain* swapChain) override;

        IBuffer* CreateBuffer(const BufferDesc& desc, const void* initialData = nullptr) override;
        void DestroyBuffer(IBuffer* buffer) override;

        ITexture* CreateTexture(const TextureDesc& desc, const void* initialData = nullptr) override;
        void DestroyTexture(ITexture* texture) override;

        IShader* CreateShader(const ShaderDesc& desc) override;
        void DestroyShader(IShader* shader) override;

        IPipelineState* CreatePipelineState(const PipelineStateDesc& desc) override;
        void DestroyPipelineState(IPipelineState* pipelineState) override;

        ICommandList* CreateCommandList() override;
        void DestroyCommandList(ICommandList* commandList) override;

        void ExecuteCommandLists(ICommandList* const* commandLists, uint32_t count) override;

        IFence* CreateFence() override;
        void DestroyFence(IFence* fence) override;

        void WaitForIdle() override;

        GraphicsAPI GetAPI() const override { return GraphicsAPI::DirectX12; }
        const DeviceFeatures& GetFeatures() const override { return m_features; }
        void* GetNativeDevice() const override { return m_device.Get(); }

        // D3D12-specific accessors
        ID3D12Device* GetD3DDevice() const { return m_device.Get(); }
        ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue.Get(); }

    private:
        bool CreateD3DDevice(const DeviceCreationParams& params);
        void DetectDeviceFeatures();
        void CreateCommandQueue();
        void CreateDescriptorHeaps();

        // D3D12 objects
        Microsoft::WRL::ComPtr<ID3D12Device> m_device;
        Microsoft::WRL::ComPtr<IDXGIFactory6> m_factory;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
        
        // Descriptor heaps
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_samplerHeap;
        
        // Descriptor sizes
        uint32_t m_rtvDescriptorSize = 0;
        uint32_t m_dsvDescriptorSize = 0;
        uint32_t m_srvDescriptorSize = 0;
        uint32_t m_samplerDescriptorSize = 0;
        
        // Device features
        DeviceFeatures m_features;
        
        // Window parameters
        void* m_nativeWindow = nullptr;
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        
        bool m_initialized = false;
    };
}