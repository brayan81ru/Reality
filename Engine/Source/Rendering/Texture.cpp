// Texture.cpp
#include "Texture.h"
#include "Renderer.h"
#include "Core/Log.h"
#include <stb_image.h>

namespace Reality {
    Texture* Texture::Create(const std::string& name, const Desc& desc) {
        return new Texture(name, desc);
    }

    Texture* Texture::LoadFromFile(const std::string& filePath, bool generateMips) {
        // Load image using stb_image
        int width, height, channels;
        stbi_uc* imageData = stbi_load(filePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

        if (!imageData) {
            RLOG_ERROR("Failed to load texture: %s", filePath.c_str());
            return nullptr;
        }

        // Determine format based on number of channels
        Format format = Format::RGBA8_UNORM;

        // Create texture description
        Desc desc;
        desc.type = Type::TEXTURE_2D;
        desc.format = format;
        desc.width = static_cast<uint32_t>(width);
        desc.height = static_cast<uint32_t>(height);
        desc.mipLevels = generateMips ? 0 : 1; // 0 means generate all mip levels
        desc.generateMips = generateMips;
        desc.initData = imageData;
        desc.initDataSize = width * height * 4; // Always 4 channels (RGBA)
        desc.usage = Usage::SHADER_RESOURCE;

        // Extract filename from path
        size_t lastSlash = filePath.find_last_of("/\\");
        std::string filename = (lastSlash != std::string::npos) ? filePath.substr(lastSlash + 1) : filePath;

        // Create texture
        Texture* texture = new Texture(filename, desc);

        // Free image data
        stbi_image_free(imageData);

        return texture;
    }

    Texture* Texture::CreateRenderTarget(uint32_t width, uint32_t height, Format format, bool useStencil) {
        Desc desc;
        desc.type = Type::TEXTURE_2D;
        desc.format = format;
        desc.width = width;
        desc.height = height;
        desc.mipLevels = 1;
        desc.usage = Usage::RENDER_TARGET;

        std::string name = "RT_" + std::to_string(width) + "x" + std::to_string(height);
        return new Texture(name, desc);
    }

    Texture* Texture::CreateDepthBuffer(uint32_t width, uint32_t height, bool useStencil) {
        Desc desc;
        desc.type = Type::TEXTURE_2D;
        desc.format = useStencil ? Format::DEPTH24_STENCIL8 : Format::DEPTH32_FLOAT;
        desc.width = width;
        desc.height = height;
        desc.mipLevels = 1;
        desc.usage = useStencil ? Usage::DEPTH_STENCIL : Usage::DEPTH_STENCIL;

        std::string name = "Depth_" + std::to_string(width) + "x" + std::to_string(height);
        return new Texture(name, desc);
    }

    Texture::Texture(const std::string& name, const Desc& desc)
        : m_name(name), m_desc(desc) {
        CreateTexture();
        CreateView();
    }

    Texture::~Texture() {
        m_texture.Release();
        m_textureView.Release();
    }

    void Texture::CreateTexture() {
        Diligent::TextureDesc texDesc;
        texDesc.Name = m_name.c_str();

        // Set texture type
        switch (m_desc.type) {
            case Type::TEXTURE_2D:
                texDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
                break;
            case Type::TEXTURE_CUBE:
                texDesc.Type = Diligent::RESOURCE_DIM_TEX_CUBE;
                break;
            case Type::TEXTURE_3D:
                texDesc.Type = Diligent::RESOURCE_DIM_TEX_3D;
                break;
        }

        // Set texture dimensions
        texDesc.Width = m_desc.width;
        texDesc.Height = m_desc.height;
        texDesc.Depth = m_desc.depth;
        texDesc.MipLevels = m_desc.mipLevels;
        texDesc.ArraySize = m_desc.arraySize;
        texDesc.SampleCount = m_desc.sampleCount;

        // Set texture format
        texDesc.Format = ConvertFormat(m_desc.format);

        // Set usage flags
        texDesc.BindFlags = ConvertUsage(m_desc.usage);

        // Set misc flags
        if (m_desc.generateMips) {
            texDesc.MiscFlags = Diligent::MISC_TEXTURE_FLAG_GENERATE_MIPS;
        }

        // Create texture
        auto& renderer = Renderer::GetInstance();

        if (m_desc.initData) {
            // Create TextureData structure for initialization
            Diligent::TextureData initData;
            initData.pContext = renderer.GetContext();
            initData.pSubResources = new Diligent::TextureSubResData[1];

            // Set up the subresource data
            initData.pSubResources[0].pData = m_desc.initData;
            initData.pSubResources[0].Stride = m_desc.width * 4; // Assuming RGBA format
            initData.pSubResources[0].DepthStride = initData.pSubResources[0].Stride * m_desc.height;

            renderer.GetDevice()->CreateTexture(texDesc, &initData, &m_texture);

            // Clean up
            delete[] initData.pSubResources;
        } else {
            renderer.GetDevice()->CreateTexture(texDesc, nullptr, &m_texture);
        }

        if (!m_texture) {
            RLOG_ERROR("Failed to create texture: %s", m_name.c_str());
        }
    }

    void Texture::CreateView() {
        if (!m_texture) return;

        Diligent::TextureViewDesc viewDesc;
        viewDesc.ViewType = Diligent::TEXTURE_VIEW_SHADER_RESOURCE;
        viewDesc.TextureDim = m_texture->GetDesc().Type;

        m_texture->CreateView(viewDesc, &m_textureView);

        if (!m_textureView) {
            RLOG_ERROR("Failed to create texture view: %s", m_name.c_str());
        }
    }


    void Texture::UpdateData(const void* data, size_t dataSize, uint32_t mipLevel, uint32_t arraySlice) {
        if (!m_texture || !data) return;

        auto& renderer = Renderer::GetInstance();
        auto context = renderer.GetContext();

        Diligent::Box updateBox;
        updateBox.MinX = 0;
        updateBox.MaxX = m_desc.width;
        updateBox.MinY = 0;
        updateBox.MaxY = m_desc.height;
        updateBox.MinZ = 0;
        updateBox.MaxZ = m_desc.depth;

        // Create TextureSubResData structure
        Diligent::TextureSubResData subresourceData;
        subresourceData.pData = data;
        subresourceData.Stride = m_desc.width * 4; // Assuming RGBA format
        subresourceData.DepthStride = subresourceData.Stride * m_desc.height;

        // Update the texture using the correct method signature
        context->UpdateTexture(
            m_texture,
            mipLevel,
            arraySlice,
            updateBox,
            subresourceData,
            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
        );
    }

    void Texture::GenerateMips() {
        if (!m_texture) return;

        auto& renderer = Renderer::GetInstance();
        auto context = renderer.GetContext();

        context->GenerateMips(m_texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
    }

    Diligent::TEXTURE_FORMAT Texture::ConvertFormat(Format format) const {
        switch (format) {
            case Format::R8_UNORM:
                return Diligent::TEX_FORMAT_R8_UNORM;
            case Format::RG8_UNORM:
                return Diligent::TEX_FORMAT_RG8_UNORM;
            case Format::RGBA8_UNORM:
                return Diligent::TEX_FORMAT_RGBA8_UNORM;
            case Format::RGBA8_UNORM_SRGB:
                return Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB;
            case Format::RGBA16_FLOAT:
                return Diligent::TEX_FORMAT_RGBA16_FLOAT;
            case Format::RGBA32_FLOAT:
                return Diligent::TEX_FORMAT_RGBA32_FLOAT;
            case Format::R32_FLOAT:
                return Diligent::TEX_FORMAT_R32_FLOAT;
            case Format::DEPTH32_FLOAT:
                return Diligent::TEX_FORMAT_D32_FLOAT;
            case Format::DEPTH24_STENCIL8:
                return Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
            default:
                return Diligent::TEX_FORMAT_UNKNOWN;
        }
    }

    Diligent::BIND_FLAGS Texture::ConvertUsage(Usage usage) const {
        switch (usage) {
            case Usage::SHADER_RESOURCE:
                return Diligent::BIND_SHADER_RESOURCE;
            case Usage::RENDER_TARGET:
                return Diligent::BIND_RENDER_TARGET;
            case Usage::DEPTH_STENCIL:
                return Diligent::BIND_DEPTH_STENCIL;
            case Usage::UNORDERED_ACCESS:
                return Diligent::BIND_UNORDERED_ACCESS;
            default:
                return Diligent::BIND_SHADER_RESOURCE;
        }
    }
}