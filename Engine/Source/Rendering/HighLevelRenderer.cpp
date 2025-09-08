#include "HighLevelRenderer.h"
#include <cassert>

namespace Reality {
    HighLevelRenderer::HighLevelRenderer(IGraphicsDevice* device)
        : m_device(device) {
    }

    HighLevelRenderer::~HighLevelRenderer() {
        if (m_isFrameActive) {
            EndFrame();
        }
    }

    bool HighLevelRenderer::Initialize(void* nativeWindow, uint32_t width, uint32_t height) {
        if (!m_device) {
            return false;
        }

        m_width = width;
        m_height = height;

        // Create swap chain
        SwapChainDesc swapDesc;
        swapDesc.nativeWindow = nativeWindow;
        swapDesc.width = width;
        swapDesc.height = height;
        swapDesc.bufferCount = 2;

        m_swapChain = SwapChainPtr(m_device->CreateSwapChain(swapDesc), ResourceDeleter<ISwapChain>(m_device));
        if (!m_swapChain) {
            return false;
        }

        // Create command list
        m_currentCommandList = CommandListPtr(m_device->CreateCommandList(), ResourceDeleter<ICommandList>(m_device));
        if (!m_currentCommandList) {
            return false;
        }

        return true;
    }

    void HighLevelRenderer::BeginFrame() {
        assert(!m_isFrameActive && "Frame already in progress");
        m_isFrameActive = true;

        // Reset command list
        m_currentCommandList->Reset();
    }

    void HighLevelRenderer::EndFrame() {
        assert(m_isFrameActive && "No frame in progress");
        m_isFrameActive = false;

        // Close command list
        m_currentCommandList->Close();

        // Execute command list
        ICommandList* commandLists[] = { m_currentCommandList.get() };
        m_device->ExecuteCommandLists(commandLists, 1);
    }

    void HighLevelRenderer::Present() {
        if (m_swapChain) {
            m_swapChain->Present(1);
        }
    }

    void HighLevelRenderer::Clear(const Vector4& color) {
        assert(m_isFrameActive && "No frame in progress");

        if (m_swapChain) {
            ITexture* backBuffer = m_swapChain->GetBackBuffer(m_swapChain->GetCurrentBackBufferIndex());
            float clearColor[] = { color.x, color.y, color.z, color.w };
            m_currentCommandList->ClearRenderTargetView(backBuffer, clearColor);
        }
    }

    void HighLevelRenderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
        assert(m_isFrameActive && "No frame in progress");

        Viewport viewport(static_cast<float>(x), static_cast<float>(y),
                         static_cast<float>(width), static_cast<float>(height));
        m_currentCommandList->RSSetViewports(1, &viewport);
    }

    void HighLevelRenderer::SetScissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom) {
        assert(m_isFrameActive && "No frame in progress");

        Rect rect(static_cast<int32_t>(left), static_cast<int32_t>(top),
                 static_cast<int32_t>(right), static_cast<int32_t>(bottom));
        m_currentCommandList->RSSetScissorRects(1, &rect);
    }

    BufferPtr HighLevelRenderer::CreateVertexBuffer(const void* data, uint32_t size, uint32_t stride) {
        BufferDesc desc;
        desc.size = size;
        desc.stride = stride;
        desc.usage = ResourceUsage::Default;
        desc.bindFlags = BufferBindFlags::VertexBuffer;

        return BufferPtr(m_device->CreateBuffer(desc, data), ResourceDeleter<IBuffer>(m_device));
    }

    BufferPtr HighLevelRenderer::CreateIndexBuffer(const void* data, uint32_t size) {
        BufferDesc desc;
        desc.size = size;
        desc.stride = sizeof(uint32_t); // Assuming 32-bit indices
        desc.usage = ResourceUsage::Default;
        desc.bindFlags = BufferBindFlags::IndexBuffer;

        return BufferPtr(m_device->CreateBuffer(desc, data), ResourceDeleter<IBuffer>(m_device));
    }

    TexturePtr HighLevelRenderer::CreateTextureFromFile(const std::string& filename) {
        // This would need to be implemented with a proper image loading library
        (void)filename;
        return nullptr;
    }

    TexturePtr HighLevelRenderer::CreateTexture2D(uint32_t width, uint32_t height, Format format, const void* data) {
        TextureDesc desc;
        desc.type = ResourceType::Texture2D;
        desc.width = width;
        desc.height = height;
        desc.depth = 1;
        desc.mipLevels = 1;
        desc.arraySize = 1;
        desc.format = format;
        desc.usage = ResourceUsage::Default;
        desc.bindFlags = TextureBindFlags::ShaderResource;

        return TexturePtr(m_device->CreateTexture(desc, data), ResourceDeleter<ITexture>(m_device));
    }

    ShaderPtr HighLevelRenderer::CreateShaderFromFile(const std::string& filename, ShaderType type, const std::string& entryPoint) {
        // This would need to be implemented with a proper file reading and shader compilation
        (void)filename;
        (void)entryPoint;

        ShaderDesc desc;
        desc.type = type;

        return ShaderPtr(m_device->CreateShader(desc), ResourceDeleter<IShader>(m_device));
    }

    PipelineStatePtr HighLevelRenderer::CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) {
        PipelineStateDesc psoDesc;
        psoDesc.vertexShader = desc.vertexShader;
        psoDesc.pixelShader = desc.pixelShader;
        psoDesc.primitiveTopology = desc.topology;

        // Set default states based on the simplified description
        if (desc.depthTest) {
            psoDesc.depthStencilState.depthEnable = true;
            psoDesc.depthStencilState.depthWriteMask = desc.depthWrite;
        } else {
            psoDesc.depthStencilState.depthEnable = false;
            psoDesc.depthStencilState.depthWriteMask = false;
        }

        if (desc.cullBack) {
            psoDesc.rasterizerState.cullMode = CullMode::Back;
        } else {
            psoDesc.rasterizerState.cullMode = CullMode::None;
        }

        return PipelineStatePtr(m_device->CreatePipelineState(psoDesc), ResourceDeleter<IPipelineState>(m_device));
    }

    void HighLevelRenderer::Draw(uint32_t vertexCount, uint32_t instanceCount) {
        assert(m_isFrameActive && "No frame in progress");
        m_currentCommandList->Draw(vertexCount, instanceCount);
    }

    void HighLevelRenderer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount) {
        assert(m_isFrameActive && "No frame in progress");
        m_currentCommandList->DrawIndexed(indexCount, instanceCount);
    }

    void HighLevelRenderer::SetVertexBuffer(uint32_t slot, IBuffer* buffer) {
        assert(m_isFrameActive && "No frame in progress");
        IBuffer* buffers[] = { buffer };
        m_currentCommandList->SetVertexBuffers(buffers, slot, 1);
    }

    void HighLevelRenderer::SetIndexBuffer(IBuffer* buffer) {
        assert(m_isFrameActive && "No frame in progress");
        m_currentCommandList->SetIndexBuffer(buffer);
    }

    void HighLevelRenderer::SetPipelineState(IPipelineState* pso) {
        assert(m_isFrameActive && "No frame in progress");
        m_currentCommandList->SetPipelineState(pso);
    }

    void HighLevelRenderer::Resize(uint32_t width, uint32_t height) {
        if (m_swapChain) {
            m_swapChain->Resize(width, height);
            m_width = width;
            m_height = height;
        }
    }

    void HighLevelRenderer::SetVSync(bool enabled) {
        if (m_swapChain) {
            m_swapChain->SetVSync(enabled);
        }
    }
}