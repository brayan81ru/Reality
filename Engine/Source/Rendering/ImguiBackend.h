#pragma once
#include <RefCntAutoPtr.hpp>
#include <ImGuiImplDiligent.hpp>
#include <SwapChain.h>

namespace Reality {

    class ImguiBackend {
    public:
        ImguiBackend();
        ~ImguiBackend();

        bool Initialize(
            Diligent::IRenderDevice* device,
            Diligent::IDeviceContext* context,
            const Diligent::ISwapChain* swapChain,
            const char* fontPath = nullptr
        );

        void BeginFrame(const Diligent::ISwapChain *swapChain) const;

        void EndFrame(Diligent::IDeviceContext *context) const;

        void Shutdown();

        //-void ProcessSDLEvent(const SDL_Event *event) const;

    private:
        std::unique_ptr<Diligent::ImGuiImplDiligent> m_ImGuiImpl;
        bool m_Initialized = false;
    };

} // namespace REngine