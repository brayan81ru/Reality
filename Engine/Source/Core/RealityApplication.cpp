#include "RealityApplication.h"
#include <Rendering/Renderer.h>
#include <Rendering/DisplayManager.h>
#include <Platform/Window.h>
#include <Core/Timer.h>

#include "Log.h"

namespace Reality {
    bool RealityApplication::IsRunning() const {
        return m_window->IsRunning();
    }

    void RealityApplication::Initialize(const std::string &title, const int width, const int height) {
        Timer::Init();

        m_window = new Window(title, width, height);

        const auto currentMode = DisplayManager::GetCurrentMode();
        RLOG_INFO("Pixel format %s",SDL_GetPixelFormatName(currentMode.format));
        /*
        DisplayManager::ApplyDisplayMode(m_window,currentMode,FullScreenMode::ExclusiveFullScreen);
        */

        m_renderer = &Renderer::GetInstance();

        m_renderer->Initialize(RenderAPI::Direct3D12, m_window);
    }

    void RealityApplication::Update() const {
        m_window->Run();
        Timer::Update();
        m_renderer->Clear();
        m_renderer->RenderStatsUI(Timer::GetFPS(),Timer::GetFrameTimeMS(), m_renderer->GetVSync());
        m_renderer->ProcessStatsUIEvents(m_window->SDL_GetEvent());
    }

    void RealityApplication::Frame() const {
        m_renderer->Frame();
    }

    void RealityApplication::Shutdown() const {
        m_renderer->~Renderer();
    }
}
