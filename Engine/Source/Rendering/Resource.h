#pragma once
#include "GraphicsDevice.h"
#include <memory>
#include <functional>

namespace Reality {
    // Custom deleter for resources
    template<typename T>
    struct ResourceDeleter {
        IGraphicsDevice* device = nullptr;
        
        ResourceDeleter() = default;
        explicit ResourceDeleter(IGraphicsDevice* dev) : device(dev) {}
        
        void operator()(T* resource) const {
            if (device && resource) {
                // Call appropriate destroy method based on resource type
                if constexpr (std::is_same_v<T, IBuffer>) {
                    device->DestroyBuffer(resource);
                } else if constexpr (std::is_same_v<T, ITexture>) {
                    device->DestroyTexture(resource);
                } else if constexpr (std::is_same_v<T, IShader>) {
                    device->DestroyShader(resource);
                } else if constexpr (std::is_same_v<T, IPipelineState>) {
                    device->DestroyPipelineState(resource);
                } else if constexpr (std::is_same_v<T, ICommandList>) {
                    device->DestroyCommandList(resource);
                } else if constexpr (std::is_same_v<T, IFence>) {
                    device->DestroyFence(resource);
                } else if constexpr (std::is_same_v<T, ISwapChain>) {
                    device->DestroySwapChain(resource);
                }
            }
        }
    };

    // Smart pointer types
    using DevicePtr = std::unique_ptr<IGraphicsDevice, ResourceDeleter<IGraphicsDevice>>;
    using SwapChainPtr = std::unique_ptr<ISwapChain, ResourceDeleter<ISwapChain>>;
    using BufferPtr = std::unique_ptr<IBuffer, ResourceDeleter<IBuffer>>;
    using TexturePtr = std::unique_ptr<ITexture, ResourceDeleter<ITexture>>;
    using ShaderPtr = std::unique_ptr<IShader, ResourceDeleter<IShader>>;
    using PipelineStatePtr = std::unique_ptr<IPipelineState, ResourceDeleter<IPipelineState>>;
    using CommandListPtr = std::unique_ptr<ICommandList, ResourceDeleter<ICommandList>>;
    using FencePtr = std::unique_ptr<IFence, ResourceDeleter<IFence>>;

    // Base Buffer Implementation
    class BufferBase : public IBuffer {
    protected:
        BufferDesc m_desc;
        void* m_mappedData = nullptr;
        IGraphicsDevice* m_device = nullptr;

    public:
        BufferBase(const BufferDesc& desc, IGraphicsDevice* device) 
            : m_desc(desc), m_device(device) {}
        virtual ~BufferBase() = default;

        // IBuffer interface
        uint32_t GetSize() const override { return m_desc.size; }
        uint32_t GetStride() const override { return m_desc.stride; }
        ResourceUsage GetUsage() const override { return m_desc.usage; }
    };

    // Base Texture Implementation
    class TextureBase : public ITexture {
    protected:
        TextureDesc m_desc;
        IGraphicsDevice* m_device = nullptr;

    public:
        TextureBase(const TextureDesc& desc, IGraphicsDevice* device) 
            : m_desc(desc), m_device(device) {}
        virtual ~TextureBase() = default;

        // ITexture interface
        uint32_t GetWidth() const override { return m_desc.width; }
        uint32_t GetHeight() const override { return m_desc.height; }
        uint32_t GetDepth() const override { return m_desc.depth; }
        uint32_t GetMipLevels() const override { return m_desc.mipLevels; }
        uint32_t GetArraySize() const override { return m_desc.arraySize; }
        Format GetFormat() const override { return m_desc.format; }
        ResourceType GetType() const override { return m_desc.type; }
        ResourceUsage GetUsage() const override { return m_desc.usage; }
    };

    // Base Shader Implementation
    class ShaderBase : public IShader {
    protected:
        ShaderDesc m_desc;
        IGraphicsDevice* m_device = nullptr;

    public:
        ShaderBase(const ShaderDesc& desc, IGraphicsDevice* device) 
            : m_desc(desc), m_device(device) {}
        virtual ~ShaderBase() = default;

        // IShader interface
        ShaderType GetType() const override { return m_desc.type; }
        const std::string& GetSource() const override { return m_desc.source; }
        const std::string& GetEntryPoint() const override { return m_desc.entryPoint; }
        const std::string& GetTarget() const override { return m_desc.target; }
    };

    // Base Pipeline State Implementation
    class PipelineStateBase : public IPipelineState {
    protected:
        PipelineStateDesc m_desc;
        IGraphicsDevice* m_device = nullptr;

    public:
        PipelineStateBase(const PipelineStateDesc& desc, IGraphicsDevice* device) 
            : m_desc(desc), m_device(device) {}
        virtual ~PipelineStateBase() = default;

        // IPipelineState interface
        const PipelineStateDesc& GetDesc() const override { return m_desc; }
    };

    // Base Command List Implementation
    class CommandListBase : public ICommandList {
    protected:
        bool m_isClosed = false;
        IGraphicsDevice* m_device = nullptr;

    public:
        CommandListBase(IGraphicsDevice* device) : m_device(device) {}
        virtual ~CommandListBase() = default;

        // ICommandList interface
        void Reset() override {
            m_isClosed = false;
            ResetImpl();
        }

        void Close() override {
            if (!m_isClosed) {
                CloseImpl();
                m_isClosed = true;
            }
        }

    protected:
        virtual void ResetImpl() = 0;
        virtual void CloseImpl() = 0;
    };

    // Base Fence Implementation
    class FenceBase : public IFence {
    protected:
        uint64_t m_value = 0;
        IGraphicsDevice* m_device = nullptr;

    public:
        FenceBase(IGraphicsDevice* device) : m_device(device) {}
        virtual ~FenceBase() = default;

        // IFence interface
        uint64_t GetCompletedValue() override {
            return GetCompletedValueImpl();
        }

        void Signal(uint64_t value) override {
            SignalImpl(value);
            m_value = value;
        }

        void Wait(uint64_t value) override {
            WaitImpl(value);
            m_value = value;
        }

    protected:
        virtual uint64_t GetCompletedValueImpl() = 0;
        virtual void SignalImpl(uint64_t value) = 0;
        virtual void WaitImpl(uint64_t value) = 0;
    };
}