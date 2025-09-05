#pragma once
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <RefCntAutoPtr.hpp>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace Reality {
    class Texture;

    class TextureManager {
    public:
        static TextureManager& GetInstance();

        // Initialization (call once at engine startup)
        void Initialize(Diligent::IRenderDevice* pDevice, Diligent::IDeviceContext* pContext);

        // Texture registration
        void RegisterTexture(Texture* pTexture);
        void UnregisterTexture(Texture* pTexture);

        // Bindless texture management
        uint32_t RegisterBindlessTexture(Diligent::ITextureView* pTextureView);
        void UnregisterBindlessTexture(uint32_t index);

        // Query capabilities
        bool SupportsBindless() const { return m_BindlessSupported; }
        uint32_t GetMaxBindlessTextures() const { return m_MaxBindlessTextures; }

        // Access to Diligent objects
        Diligent::IRenderDevice* GetDevice() const { return m_pDevice; }
        Diligent::IDeviceContext* GetContext() const { return m_pContext; }

        // Frame synchronization
        void BeginFrame();
        void EndFrame();

    private:
        TextureManager();
        ~TextureManager();

        // Bindless texture heap management
        void CreateBindlessResources();
        void ReleaseBindlessResources();

        Diligent::RefCntAutoPtr<Diligent::IRenderDevice> m_pDevice;
        Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_pContext;

        // Bindless texture resources
        Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_BindlessSRB;
        std::vector<Diligent::ITextureView*> m_BindlessTextures;
        std::vector<uint32_t> m_FreeBindlessIndices;

        // Tracking for frame-to-frame consistency
        std::vector<uint32_t> m_TexturesToRemove;
        std::vector<Diligent::ITextureView*> m_TexturesToAdd;

        // Capabilities
        bool m_BindlessSupported = false;
        uint32_t m_MaxBindlessTextures = 0;

        // Thread safety
        std::mutex m_Mutex;

        // Texture registry
        std::vector<Texture*> m_Textures;
    };
} // namespace REngine