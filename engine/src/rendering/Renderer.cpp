#include "Renderer.h"
#include <SDL.h>
#include <EngineFactoryD3D11.h>
#include <EngineFactoryD3D12.h>
#include <EngineFactoryOpenGL.h>
#include <EngineFactoryVk.h>
#include <RefCntAutoPtr.hpp>
#include <imgui.h>

#include "core/Log.h"

namespace Reality {

    Renderer & Renderer::GetInstance() {
        static Renderer instance;
        return instance;
    }

    void Renderer::Release() const {
        delete this;
    }

    void Renderer::Initialize(const RenderAPI RenderApi, Window *Window) {
        RLOG_INFO("Initializing renderer...");

        // Store the original windows.
        m_RealityWindow = Window;
        m_RealityWindow->SetRenderer(this);

        // get the sdlWindows from the Reality Windows.
        const auto sdlWindow = m_RealityWindow->GetNativeWindow();

        // Get the diligent compatible window.
        m_Window = Window::SDLWindowToNativeWindow(sdlWindow);

        // Store the render api.
        m_RenderAPI = RenderApi;

        switch (m_RenderAPI) {

            case RenderAPI::Direct3D11: InitializeRendererD3D11(); break;

            case RenderAPI::Direct3D12: InitializeRendererD3D12(); break;

            case RenderAPI::OpenGL: InitializeRendererOpenGL(); break;

            case RenderAPI::Vulkan: InitializeRendererVulkan(); break;

            default: {
                RLOG_ERROR("Rendering API not supported");
            };
        }

        // Initialization
        m_ImguiBackend = new ImguiBackend();
        m_ImguiBackend->Initialize(m_pDevice, m_pImmediateContext, m_pSwapChain);
        RLOG_INFO("Renderer initialized successfully");
    }

    Renderer::~Renderer() {
        RLOG_INFO("Finalizing rendering system...");
        m_pSwapChain.Release();
        m_pDevice.Release();
        RLOG_INFO("Rendering system finalized successfully ");
    }

    void Renderer::RenderStatsUI(const float fps, const float frameTime, bool vSync) const {
        m_ImguiBackend->BeginFrame(m_pSwapChain);

        ImGui::Begin("STATS",nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::Text("%s",m_RenderAPI.ToString());
        ImGui::Text("V-Sync: %s",vSync ? "Enabled" : "Disabled");
        ImGui::Text("FPS: %.2f", fps);
        ImGui::Text("Frametime(ms): %.2f", frameTime);
        ImGui::End();
        m_ImguiBackend->EndFrame(m_pImmediateContext);
    }

    void Renderer::ProcessStatsUIEvents(const SDL_Event *event) const {
        m_ImguiBackend->ProcessSDLEvent(event);
    }

    void Renderer::Clear() const {

        constexpr float ClearColor[] = {0.f, 0.f, 0.f, 1.0f};

        // 1. Get views
        Diligent::ITextureView* pRTV = m_pSwapChain->GetCurrentBackBufferRTV();
        Diligent::ITextureView* pDSV = m_pSwapChain->GetDepthBufferDSV();

        // 2. Bind targets FIRST (optimal path)
        m_pImmediateContext->SetRenderTargets(
            1, &pRTV, pDSV,
            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
        );

        // 3. Now clear (will use fast path)
        m_pImmediateContext->ClearRenderTarget(pRTV, ClearColor,
            Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE); // No need for another transition

        m_pImmediateContext->ClearDepthStencil(pDSV,
            Diligent::CLEAR_DEPTH_FLAG, 1.f, 0,
            Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);
    }

    void Renderer::Frame() const {
        m_pSwapChain->Present(m_Vsync ? 1 : 0);
    }

    void Renderer::SetVSync(const bool vsync) {
        m_Vsync = vsync;
    }

    void Renderer::InitializeRendererD3D11() {
        RLOG_INFO("Initializing D3D11 RHI...");
        const Diligent::EngineD3D11CreateInfo EngineCI;
        auto* pFactoryD3D11 = Diligent::GetEngineFactoryD3D11();
        pFactoryD3D11->CreateDeviceAndContextsD3D11(EngineCI, &m_pDevice, &m_pImmediateContext);
        const Diligent::Win32NativeWindow Window{m_Window};
        pFactoryD3D11->CreateSwapChainD3D11(m_pDevice, m_pImmediateContext, SCDesc, Diligent::FullScreenModeDesc{}, Window, &m_pSwapChain);
        m_pEngineFactory = pFactoryD3D11;
        RLOG_INFO("D3D11 RHI initialized successfully");
    }

    void Renderer::InitializeRendererD3D12() {
        RLOG_INFO("Initializing D3D12 RHI...");
        const Diligent::EngineD3D12CreateInfo EngineCI;
        auto* pFactoryD3D12 = Diligent::GetEngineFactoryD3D12();
        pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &m_pDevice, &m_pImmediateContext);
        const Diligent::Win32NativeWindow Window{m_Window};
        pFactoryD3D12->CreateSwapChainD3D12(m_pDevice, m_pImmediateContext, SCDesc, Diligent::FullScreenModeDesc{}, Window, &m_pSwapChain);
        m_pEngineFactory = pFactoryD3D12;
        RLOG_INFO("D3D12 RHI initialized successfully");
    }

    void Renderer::InitializeRendererVulkan() {
        RLOG_INFO("Initializing Vulkan RHI...");
        const Diligent::EngineVkCreateInfo EngineCI;
        auto* pFactoryVk = Diligent::GetEngineFactoryVk();
        pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &m_pDevice, &m_pImmediateContext);
        const Diligent::Win32NativeWindow Window{m_Window};
        pFactoryVk->CreateSwapChainVk(m_pDevice, m_pImmediateContext, SCDesc, Window, &m_pSwapChain);
        m_pEngineFactory = pFactoryVk;
        RLOG_INFO("Vulkan RHI initialized successfully");
    }

    void Renderer::InitializeRendererOpenGL() {
        RLOG_INFO("Initializing OpenGL RHI...");
        auto* pFactoryOpenGL = Diligent::GetEngineFactoryOpenGL();
        Diligent::EngineGLCreateInfo EngineCI;
        const Diligent::Win32NativeWindow Window{m_Window};
        EngineCI.Window.hWnd = Window.hWnd;
        pFactoryOpenGL->CreateDeviceAndSwapChainGL(EngineCI, &m_pDevice, &m_pImmediateContext,SCDesc, &m_pSwapChain);
        m_pEngineFactory = pFactoryOpenGL;
        RLOG_INFO("OpenGL RHI initialized successfully");
    }

    void Renderer::RecreateSwapChain() {
        // Get current window dimensions
        uint32_t newWidth, newHeight;
        GetWindowSize(&newWidth, &newHeight);

        // Only recreate if dimensions changed
        if (m_pSwapChain && (newWidth == SCDesc.Width && newHeight == SCDesc.Height)) {
            return;
        }

        // Update swap chain description with new dimensions
        SCDesc.Width = newWidth;
        SCDesc.Height = newHeight;

        // Release current swap chain
        if (m_pSwapChain) {
            m_pSwapChain->Resize(newWidth, newHeight);
        }
    }

    void Renderer::GetWindowSize(uint32_t *width, uint32_t *height) const {
        // get the sdlWindows from the Reality Windows.
        const auto sdlWindow = m_RealityWindow->GetNativeWindow();
        int w, h;
        SDL_GetWindowSize(sdlWindow, &w, &h);
        *width = static_cast<uint32_t>(w);
        *height = static_cast<uint32_t>(h);
    }


}
