#pragma once

#include <NativeWindow.h>
#include <RefCntAutoPtr.hpp>
#include <RenderDevice.h>
#include <rendering/ImguiBackend.h>
#include <rendering/DisplayManager.h>

namespace Reality {
    struct RenderAPI {
        // The actual enum values
        enum Value {
            OpenGL,
            Direct3D11,
            Direct3D12,
            Vulkan,
            Count  // Useful for iteration/validation
        };

        Value value;

        // Implicit conversion
        constexpr RenderAPI(Value v = OpenGL) : value(v) {}
        operator Value() const { return value; }

        // Disallow implicit conversions from other types
        explicit operator bool() = delete;

        // String conversion
        const char* ToString() const {
            static const char* names[] = {
                "OpenGL",
                "Direct3D11",
                "Direct3D12",
                "Vulkan"
            };
            return (value < Count) ? names[value] : "Unknown";
        }

        // Optional bonus methods
        static constexpr size_t CountValues() { return Count; }
        static const char* GetName(const size_t index) {
            static const char* names[] = {
                "OpenGL",
                "Direct3D11",
                "Direct3D12",
                "Vulkan"
            };
            return (index < Count) ? names[index] : "Unknown";
        }
    };

    class Renderer {
    public:

        void Release() const;

        Renderer(RenderAPI RenderApi, Window* Window);

        ~Renderer();

        void RenderStatsUI(float fps, float frameTime, bool vSync) const;

        void ProcessStatsUIEvents(const SDL_Event *event) const;

        void Clear() const;

        void Frame() const;

        void SetVSync(bool vsync);

        [[nodiscard]] bool GetVSync() const { return m_Vsync;}

        void RecreateSwapChain();

        [[nodiscard]] Diligent::RefCntAutoPtr<Diligent::IRenderDevice> GetDevice() const { return m_pDevice;}
        [[nodiscard]] Diligent::RefCntAutoPtr<Diligent::IDeviceContext> GetContext() const { return m_pImmediateContext;}
        [[nodiscard]] Diligent::RefCntAutoPtr<Diligent::ISwapChain> GetSwapChain() const { return m_pSwapChain;}
        [[nodiscard]] Diligent::IEngineFactory* GetEngineFactory() const { return m_pEngineFactory; };
        [[nodiscard]] Diligent::RefCntAutoPtr<Diligent::IPipelineState> GetPSO() const { return m_pPSO; };
        void GetWindowSize(uint32_t *width, uint32_t *height) const;

    private:
        bool m_Vsync = true;
        Window* m_RealityWindow;
        Diligent::NativeWindow m_Window;
        RenderAPI m_RenderAPI = RenderAPI::OpenGL;
        Diligent::SwapChainDesc SCDesc;
        Diligent::RefCntAutoPtr<Diligent::IRenderDevice>  m_pDevice;
        Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_pImmediateContext;
        Diligent::RefCntAutoPtr<Diligent::ISwapChain>     m_pSwapChain;
        ImguiBackend *m_ImguiBackend;
        Diligent::IEngineFactory *m_pEngineFactory{};
        Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pPSO;

        void InitializeRendererD3D11();

        void InitializeRendererD3D12();

        void InitializeRendererVulkan();

        void InitializeRendererOpenGL();

    };
}
