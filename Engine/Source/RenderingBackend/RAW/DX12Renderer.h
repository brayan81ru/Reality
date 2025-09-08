#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <string>

namespace Reality {
    class DX12Renderer {
    public:
        DX12Renderer(HWND hwnd, int width, int height);
        ~DX12Renderer();

        void Render();
        void Resize(int width, int height);

    private:
        void Initialize();
        void CreateRootSignature();
        void CreatePipelineState();
        void CreateVertexBuffer();
        void CreateSyncObjects();
        void CreateRenderTargets();
        void WaitForPreviousFrame();
        void HandleError(HRESULT hr, const std::string& message);

        HWND m_hwnd;
        int m_width;
        int m_height;
        bool m_initialized;

        static const int FrameCount = 2;

        Microsoft::WRL::ComPtr<ID3D12Device> m_device;
        Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

        Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
        UINT m_rtvDescriptorSize;

        Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
        UINT64 m_fenceValues[FrameCount];
        HANDLE m_fenceEvent;

        Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
        D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;

        D3D12_VIEWPORT m_viewport;
        D3D12_RECT m_scissorRect;

        UINT m_frameIndex;
    };
} // namespace Reality