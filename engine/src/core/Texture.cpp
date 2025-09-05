#include "Texture.h"
#include <Core/TextureManager.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <DiligentTools/TextureLoader/interface/TextureUtilities.h>
#include <vector>

namespace Reality {
    Texture::Texture() {
        TextureManager::GetInstance().RegisterTexture(this);
    }

    Texture::~Texture() {
        TextureManager::GetInstance().UnregisterTexture(this);
        ReleaseResources();
    }

    bool Texture::LoadFromFile(const std::string& path, bool isSRGB) {
        try {
            ReleaseResources();

            Diligent::TextureLoadInfo loadInfo;
            loadInfo.IsSRGB = isSRGB;

            auto* pDevice = TextureManager::GetInstance().GetDevice();
            Diligent::CreateTextureFromFile(path.c_str(), loadInfo, pDevice, &m_Texture);

            if (!m_Texture) return false;

            m_TextureSRV = m_Texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
            if (!m_TextureSRV) return false;

            m_Width = m_Texture->GetDesc().Width;
            m_Height = m_Texture->GetDesc().Height;
            m_Format = static_cast<Format>(m_Texture->GetDesc().Format);
            m_Path = path;

            CreateInternalResources();
            return true;
        } catch (...) {
            return false;
        }
    }

    bool Texture::Create(uint32_t width, uint32_t height, Format format, const void* data) {
        ReleaseResources();

        m_Width = width;
        m_Height = height;
        m_Format = format;
        m_Path = "ProceduralTexture";

        Diligent::TextureDesc texDesc;
        texDesc.Name = m_Path.c_str();
        texDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.Format = static_cast<Diligent::TEXTURE_FORMAT>(format);
        texDesc.MipLevels = 1;
        texDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
        texDesc.Usage = data ? Diligent::USAGE_IMMUTABLE : Diligent::USAGE_DEFAULT;

        auto* pDevice = TextureManager::GetInstance().GetDevice();

        if (data) {
            Diligent::TextureSubResData subResData(data, width * GetBytesPerPixel());
            Diligent::TextureData texData(&subResData, 1);
            pDevice->CreateTexture(texDesc, &texData, &m_Texture);
        } else {
            pDevice->CreateTexture(texDesc, nullptr, &m_Texture);
        }

        if (!m_Texture) return false;

        m_TextureSRV = m_Texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
        CreateInternalResources();

        return true;
    }

    bool Texture::CreateSolidColor(const Color& color, uint32_t size) {
        std::vector<uint8_t> data(size * size * 4);
        for (uint32_t i = 0; i < size * size; ++i) {
            data[i * 4 + 0] = static_cast<uint8_t>(color.r * 255);
            data[i * 4 + 1] = static_cast<uint8_t>(color.g * 255);
            data[i * 4 + 2] = static_cast<uint8_t>(color.b * 255);
            data[i * 4 + 3] = static_cast<uint8_t>(color.a * 255);
        }
        return Create(size, size, Format::RGBA8_UNORM, data.data());
    }

    void Texture::Bind(const char* varName, Diligent::IShaderResourceBinding* pSRB) const {
        if (pSRB && varName) {
            pSRB->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, varName)->Set(m_TextureSRV);
        }
    }

    void Texture::SetWrapMode(WrapMode mode) {
        m_WrapMode = mode;
        // TODO: Implement sampler state update
    }

    void Texture::SetFilterMode(FilterMode mode) {
        m_FilterMode = mode;
        // TODO: Implement sampler state update
    }

    void Texture::SetAnisotropy(uint32_t level) {
        m_Anisotropy = level;
        // TODO: Implement sampler state update
    }

    void Texture::CreateInternalResources() {
        if (TextureManager::GetInstance().SupportsBindless()) {
            m_BindlessIndex = TextureManager::GetInstance().RegisterBindlessTexture(m_TextureSRV);
        }
    }

    void Texture::ReleaseResources() {
        if (m_BindlessIndex != UINT32_MAX) {
            TextureManager::GetInstance().UnregisterBindlessTexture(m_BindlessIndex);
            m_BindlessIndex = UINT32_MAX;
        }
        m_TextureSRV.Release();
        m_Texture.Release();
    }

    uint32_t Texture::GetBytesPerPixel() const {
        switch (m_Format) {
            case Format::RGBA8_UNORM: return 4;
            case Format::RGBA16_FLOAT: return 8;
            case Format::RGBA32_FLOAT: return 16;
            case Format::R8_UNORM: return 1;
            case Format::RGB10A2_UNORM: return 4;
            case Format::D32_FLOAT: return 4;
            default: return 4;
        }
    }
} // namespace REngine