#pragma once
#include "GraphicsDevice.h"
#include "Resource.h"
#include <Core/MathF.h>
#include <memory>
#include <string>

namespace Reality {
    struct GraphicsPipelineDesc;

    // High-level renderer that uses the low-level backend
    class HighLevelRenderer {
    public:
        explicit HighLevelRenderer(IGraphicsDevice* device);
        ~HighLevelRenderer();

        // Initialization
        bool Initialize(void* nativeWindow, uint32_t width, uint32_t height);

        // Frame control
        void BeginFrame();
        void EndFrame();
        void Present();

        // Rendering operations
        void Clear(const Vector4& color = Vector4(0.0f, 0.0f, 0.0f, 1.0f));
        void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        void SetScissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);

        // Simplified resource creation
        BufferPtr CreateVertexBuffer(const void* data, uint32_t size, uint32_t stride);
        BufferPtr CreateIndexBuffer(const void* data, uint32_t size);
        TexturePtr CreateTextureFromFile(const std::string& filename);
        TexturePtr CreateTexture2D(uint32_t width, uint32_t height, Format format, const void* data = nullptr);
        ShaderPtr CreateShaderFromFile(const std::string& filename, ShaderType type, const std::string& entryPoint = "main");

        // Pipeline management
        PipelineStatePtr CreateGraphicsPipeline(const GraphicsPipelineDesc& desc);

        // Drawing
        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1);
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1);

        // Resource binding
        void SetVertexBuffer(uint32_t slot, IBuffer* buffer);
        void SetIndexBuffer(IBuffer* buffer);
        void SetPipelineState(IPipelineState* pso);

        // Window management
        void Resize(uint32_t width, uint32_t height);
        void SetVSync(bool enabled);

        // Access to low-level API for advanced users
        IGraphicsDevice* GetDevice() { return m_device; }
        ISwapChain* GetSwapChain() { return m_swapChain.get(); }
        ICommandList* GetCurrentCommandList() { return m_currentCommandList.get(); }

    private:
        IGraphicsDevice* m_device;
        SwapChainPtr m_swapChain;
        CommandListPtr m_currentCommandList;

        // Default objects
        PipelineStatePtr m_defaultPipeline;

        // Frame state
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        bool m_isFrameActive = false;
    };

    // Simplified pipeline description
    struct GraphicsPipelineDesc {
        ShaderDesc vertexShader;
        ShaderDesc pixelShader;
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;
        bool depthTest = true;
        bool depthWrite = true;
        bool cullBack = true;
    };
}