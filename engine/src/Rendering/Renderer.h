#pragma once
#include <NativeWindow.h>
#include <RefCntAutoPtr.hpp>
#include <RenderDevice.h>
#include <Rendering/ImguiBackend.h>
#include <Rendering/DisplayManager.h>
#include "BasicMath.hpp"

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

        static Renderer& GetInstance();

        void Initialize(RenderAPI RenderApi, Window* Window);

        Renderer() = default;

        ~Renderer();

        void RenderStatsUI(float fps, float frameTime, bool vSync) const;

        void ProcessStatsUIEvents(const SDL_Event *event) const;

        void Clear() const;

        void Frame() const;

        void SetVSync(bool vsync);

        [[nodiscard]] bool GetVSync() const { return m_Vsync;}

        void RecreateSwapChain();

        [[nodiscard]] RefCntAutoPtr<IRenderDevice> GetDevice() const { return m_pDevice;}
        [[nodiscard]] RefCntAutoPtr<IDeviceContext> GetContext() const { return m_pImmediateContext;}
        [[nodiscard]] RefCntAutoPtr<ISwapChain> GetSwapChain() const { return m_pSwapChain;}
        [[nodiscard]] IEngineFactory* GetEngineFactory() const { return m_pEngineFactory; };
        [[nodiscard]] RefCntAutoPtr<IPipelineState> GetPSO() const { return m_pPSO; };
        [[nodiscard]] Matrix4x4<float> GetWorldProjectionMatrix() const { return m_WorldViewProjMatrix; }
        void GetWindowSize(uint32_t *width, uint32_t *height) const;

        void SetWorldProjectionMatrix(const Matrix4x4<float> &WorldViewProjMatrix) { m_WorldViewProjMatrix = WorldViewProjMatrix; }

    private:
        bool m_Vsync = true;
        Window* m_RealityWindow{};
        NativeWindow m_Window;
        RenderAPI m_RenderAPI = RenderAPI::OpenGL;
        SwapChainDesc SCDesc;
        RefCntAutoPtr<IRenderDevice>  m_pDevice;
        RefCntAutoPtr<IDeviceContext> m_pImmediateContext;
        RefCntAutoPtr<ISwapChain>     m_pSwapChain;
        ImguiBackend *m_ImguiBackend{};
        IEngineFactory *m_pEngineFactory{};
        RefCntAutoPtr<IPipelineState> m_pPSO;
        Matrix4x4<float> m_WorldViewProjMatrix;

        void InitializeRendererD3D11();

        void InitializeRendererD3D12();

        void InitializeRendererVulkan();

        void InitializeRendererOpenGL();

    };
}
