#pragma once

#include "GraphicsTypes.h"
#include <memory>
#include <vector>

namespace Reality {
    // Forward declarations
    class IBuffer;
    class ITexture;
    class IShader;
    class IPipelineState;
    class ICommandList;
    class IFence;

    // Graphics Device Interface
    class IGraphicsDevice {
    public:
        virtual ~IGraphicsDevice() = default;

        // Device Management
        virtual void Initialize(void* nativeWindow) = 0;
        virtual void Shutdown() = 0;
        virtual void Resize(uint32_t width, uint32_t height) = 0;

        // Resource Creation
        virtual IBuffer* CreateBuffer(const BufferDesc& desc, const void* initialData = nullptr) = 0;
        virtual ITexture* CreateTexture(const TextureDesc& desc, const void* initialData = nullptr) = 0;
        virtual IShader* CreateShader(const ShaderDesc& desc) = 0;
        virtual IPipelineState* CreatePipelineState(const PipelineStateDesc& desc) = 0;

        // Command Submission
        virtual ICommandList* CreateCommandList() = 0;
        virtual void ExecuteCommandLists(ICommandList* const* commandLists, uint32_t count) = 0;

        // Synchronization
        virtual IFence* CreateFence() = 0;
        virtual void WaitForIdle() = 0;

        // Swap Chain
        virtual void Present() = 0;
        virtual uint32_t GetBackBufferIndex() const = 0;
        virtual ITexture* GetBackBuffer(uint32_t index) = 0;
    };

    // Buffer Interface
    class IBuffer {
    public:
        virtual ~IBuffer() = default;
        virtual void* Map() = 0;
        virtual void Unmap() = 0;
        virtual void UpdateData(const void* data, size_t size, size_t offset = 0) = 0;
        virtual uint32_t GetSize() const = 0;
        virtual uint32_t GetStride() const = 0;
        virtual ResourceUsage GetUsage() const = 0;
    };

    // Texture Interface
    class ITexture {
    public:
        virtual ~ITexture() = default;
        virtual void UpdateData(const void* data, uint32_t mipLevel, uint32_t arraySlice) = 0;
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual uint32_t GetDepth() const = 0;
        virtual uint32_t GetMipLevels() const = 0;
        virtual uint32_t GetArraySize() const = 0;
        virtual Format GetFormat() const = 0;
        virtual ResourceType GetType() const = 0;
        virtual ResourceUsage GetUsage() const = 0;
    };

    // Shader Interface
    class IShader {
    public:
        virtual ~IShader() = default;
        virtual ShaderType GetType() const = 0;
        virtual const std::string& GetSource() const = 0;
        virtual const std::string& GetEntryPoint() const = 0;
        virtual const std::string& GetTarget() const = 0;
    };

    // Pipeline State Interface
    class IPipelineState {
    public:
        virtual ~IPipelineState() = default;
        virtual const PipelineStateDesc& GetDesc() const = 0;
    };

    // Command List Interface
    class ICommandList {
    public:
        virtual ~ICommandList() = default;
        virtual void Reset() = 0;
        virtual void Close() = 0;
        virtual void ResourceBarrier(ITexture* resource, ResourceState before, ResourceState after) = 0;
        virtual void SetPipelineState(IPipelineState* pipeline) = 0;
        virtual void SetVertexBuffers(IBuffer* const* buffers, uint32_t startSlot, uint32_t numBuffers) = 0;
        virtual void SetIndexBuffer(IBuffer* buffer) = 0;
        virtual void SetGraphicsRootConstantBufferView(uint32_t rootIndex, IBuffer* buffer) = 0;
        virtual void SetGraphicsRootDescriptorTable(uint32_t rootIndex, IBuffer* buffer) = 0;
        virtual void Draw(uint32_t vertexCount, uint32_t instanceCount = 1) = 0;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1) = 0;
        virtual void CopyTextureRegion(ITexture* dst, ITexture* src) = 0;
        virtual void ClearRenderTargetView(ITexture* renderTarget, const float color[4]) = 0;
        virtual void ClearDepthStencilView(ITexture* depthStencil, float depth, uint8_t stencil) = 0;
        virtual void OMSetRenderTargets(uint32_t numRenderTargets, ITexture* const* renderTargets, ITexture* depthStencil) = 0;
        virtual void RSSetViewports(uint32_t numViewports, const Viewport* viewports) = 0;
        virtual void RSSetScissorRects(uint32_t numRects, const Rect* rects) = 0;
    };

    // Fence Interface
    class IFence {
    public:
        virtual ~IFence() = default;
        virtual uint64_t GetCompletedValue() = 0;
        virtual void Signal(uint64_t value) = 0;
        virtual void Wait(uint64_t value) = 0;
    };
}