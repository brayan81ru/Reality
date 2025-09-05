#include "TextureManager.h"
#include <DiligentCore/Graphics/GraphicsAccessories/interface/GraphicsAccessories.hpp>

namespace Reality {
    TextureManager::TextureManager() {
        // Initialize with null values
    }

    TextureManager::~TextureManager() {
        ReleaseBindlessResources();
    }

    TextureManager& TextureManager::GetInstance() {
        static TextureManager instance;
        return instance;
    }

    void TextureManager::Initialize(Diligent::IRenderDevice* pDevice, Diligent::IDeviceContext* pContext) {
        m_pDevice = pDevice;

        m_pContext = pContext;

        // Check bindless support by checking for dynamic shader resource binding
        const auto& deviceInfo = m_pDevice->GetDeviceInfo();
        m_BindlessSupported = deviceInfo.Features.BindlessResources;

        if (m_BindlessSupported) {
            // Conservative estimate for maximum textures
            m_MaxBindlessTextures = 1024;
            CreateBindlessResources();
        }
    }

    void TextureManager::RegisterTexture(Texture* pTexture) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Textures.push_back(pTexture);
    }

    void TextureManager::UnregisterTexture(Texture* pTexture) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Textures.erase(std::remove(m_Textures.begin(), m_Textures.end(), pTexture), m_Textures.end());
    }

    void TextureManager::CreateBindlessResources() {
        if (!m_BindlessSupported) return;

        // Initialize free indices
        m_FreeBindlessIndices.resize(m_MaxBindlessTextures);
        for (uint32_t i = 0; i < m_MaxBindlessTextures; ++i) {
            m_FreeBindlessIndices[i] = i;
        }

        m_BindlessTextures.resize(m_MaxBindlessTextures, nullptr);
    }

    void TextureManager::ReleaseBindlessResources() {
        m_BindlessSRB.Release();
        m_FreeBindlessIndices.clear();
        m_BindlessTextures.clear();
        m_TexturesToAdd.clear();
        m_TexturesToRemove.clear();
    }

    uint32_t TextureManager::RegisterBindlessTexture(Diligent::ITextureView* pTextureView) {
        if (!m_BindlessSupported || !pTextureView) {
            return UINT32_MAX;
        }

        std::lock_guard<std::mutex> lock(m_Mutex);

        // Check if already registered
        for (uint32_t i = 0; i < m_BindlessTextures.size(); ++i) {
            if (m_BindlessTextures[i] == pTextureView) {
                return i;
            }
        }

        if (m_FreeBindlessIndices.empty()) {
            // No more slots available
            return UINT32_MAX;
        }

        // Get a free index
        uint32_t index = m_FreeBindlessIndices.back();
        m_FreeBindlessIndices.pop_back();

        // Add to deferred update list
        m_TexturesToAdd.push_back(pTextureView);
        m_BindlessTextures[index] = pTextureView;

        return index;
    }

    void TextureManager::UnregisterBindlessTexture(uint32_t index) {
        if (!m_BindlessSupported || index >= m_BindlessTextures.size()) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_Mutex);

        // Add to deferred removal list
        m_TexturesToRemove.push_back(index);

        // Return index to free list
        m_FreeBindlessIndices.push_back(index);
        m_BindlessTextures[index] = nullptr;
    }

    void TextureManager::BeginFrame() {
        // Nothing to do here currently
    }

    void TextureManager::EndFrame() {
        if (!m_BindlessSupported) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_Mutex);

        // Process texture additions
        for (auto* pTextureView : m_TexturesToAdd) {
            // In older Diligent versions, we rely on the shader to handle the array
            // The actual binding happens at shader level
        }
        m_TexturesToAdd.clear();

        // Process texture removals
        for (uint32_t index : m_TexturesToRemove) {
            // Just mark the slot as available
        }
        m_TexturesToRemove.clear();
    }
}// namespace REngine