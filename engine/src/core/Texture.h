#pragma once
#include <DiligentCore/Graphics/GraphicsEngine/interface/Texture.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/TextureView.h>
#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>
#include <DiligentTools/TextureLoader/interface/TextureUtilities.h>
#include <Core/Color.h>
#include <string>
#include <memory>

namespace Reality {

class Texture {
public:
    enum class WrapMode { Repeat, Clamp, Mirror };
    enum class FilterMode { Linear, Nearest, Anisotropic };
    enum class Format {
        RGBA8_UNORM, RGBA16_FLOAT, RGBA32_FLOAT,
        R8_UNORM, RGB10A2_UNORM, D32_FLOAT
    };
    enum class Type { Albedo, Normal, MetallicRoughness, Emissive, Occlusion };

    Texture();
    ~Texture();

    // Creation methods
    bool LoadFromFile(const std::string& path, bool isSRGB = true);
    bool Create(uint32_t width, uint32_t height, Format format, const void* data = nullptr);
    bool CreateSolidColor(const Color& color, uint32_t size = 1);

    void Bind(const char *varName, Diligent::IShaderResourceBinding *pSRB) const;

    // Bindless support
    uint32_t GetBindlessIndex() const { return m_BindlessIndex; }
    bool IsBindless() const { return m_BindlessIndex != UINT32_MAX; }

    // Traditional binding
    void Bind(uint32_t slot) const;

    // Texture parameters
    void SetWrapMode(WrapMode mode);
    void SetFilterMode(FilterMode mode);
    void SetAnisotropy(uint32_t level);

    // Getters
    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    Format GetFormat() const { return m_Format; }
    const std::string& GetPath() const { return m_Path; }

    // Internal Diligent resources
    Diligent::ITexture* GetDiligentTexture() { return m_Texture.RawPtr(); }
    Diligent::ITextureView* GetDiligentTextureView() { return m_TextureSRV.RawPtr(); }

    //Diligent::ITextureView* GetDiligentTextureView() { return m_TextureSRV.RawPtr(); }
    const Diligent::ITextureView* GetDiligentTextureView() const { return m_TextureSRV.RawPtr(); }

private:
    friend class TextureManager;

    void CreateInternalResources();
    void ReleaseResources();
    uint32_t GetBytesPerPixel() const;

    Diligent::RefCntAutoPtr<Diligent::ITexture> m_Texture;
    Diligent::RefCntAutoPtr<Diligent::ITextureView> m_TextureSRV;

    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
    Format m_Format = Format::RGBA8_UNORM;
    std::string m_Path;

    uint32_t m_BindlessIndex = UINT32_MAX;
    WrapMode m_WrapMode = WrapMode::Repeat;
    FilterMode m_FilterMode = FilterMode::Linear;
    uint32_t m_Anisotropy = 1;
};

} // namespace REngine