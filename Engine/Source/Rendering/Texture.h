// Texture.h
#pragma once
#include <string>
#include "RefCntAutoPtr.hpp"
#include "GraphicsTypes.h"
#include "DiligentCore/Graphics/GraphicsEngine/interface/Texture.h"


namespace Reality {
    class Texture {
    public:
        // Texture types
        enum class Type {
            TEXTURE_2D,
            TEXTURE_CUBE,
            TEXTURE_3D
        };

        // Texture formats
        enum class Format {
            R8_UNORM,
            RG8_UNORM,
            RGBA8_UNORM,
            RGBA8_UNORM_SRGB,
            RGBA16_FLOAT,
            RGBA32_FLOAT,
            R32_FLOAT,
            DEPTH32_FLOAT,
            DEPTH24_STENCIL8
        };

        // Texture usage flags
        enum class Usage {
            SHADER_RESOURCE,
            RENDER_TARGET,
            DEPTH_STENCIL,
            UNORDERED_ACCESS
        };

        // Texture creation parameters
        struct Desc {
            Type type = Type::TEXTURE_2D;
            Format format = Format::RGBA8_UNORM;
            uint32_t width = 1;
            uint32_t height = 1;
            uint32_t depth = 1;
            uint32_t mipLevels = 1;
            uint32_t arraySize = 1;
            uint32_t sampleCount = 1;
            Usage usage = Usage::SHADER_RESOURCE;
            bool generateMips = true;
            const void* initData = nullptr;
            size_t initDataSize = 0;
        };

        // Factory methods
        static Texture* Create(const std::string& name, const Desc& desc);
        static Texture* LoadFromFile(const std::string& filePath, bool generateMips = true);
        static Texture* CreateRenderTarget(uint32_t width, uint32_t height, Format format, bool useStencil = false);
        static Texture* CreateDepthBuffer(uint32_t width, uint32_t height, bool useStencil = false);

        // Constructor/Destructor
        Texture(const std::string& name, const Desc& desc);
        ~Texture();

        // Getters
        const std::string& GetName() const { return m_name; }
        Diligent::ITexture* GetTexture() const { return m_texture; }
        Diligent::ITextureView* GetView() const { return m_textureView; }
        uint32_t GetWidth() const { return m_desc.width; }
        uint32_t GetHeight() const { return m_desc.height; }
        uint32_t GetDepth() const { return m_desc.depth; }
        Format GetFormat() const { return m_desc.format; }
        Type GetType() const { return m_desc.type; }

        // Update texture data
        void UpdateData(const void* data, size_t dataSize, uint32_t mipLevel = 0, uint32_t arraySlice = 0);

        // Generate mipmaps
        void GenerateMips();

    private:
        std::string m_name;
        Desc m_desc;
        Diligent::RefCntAutoPtr<Diligent::ITexture> m_texture;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> m_textureView;

        // Helper methods
        void CreateTexture();
        void CreateView();
        Diligent::TEXTURE_FORMAT ConvertFormat(Format format) const;
        Diligent::BIND_FLAGS ConvertUsage(Usage usage) const;
    };
}
