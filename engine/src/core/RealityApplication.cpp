#include "RealityApplication.h"

namespace Reality {
    bool RealityApplication::IsRunning() {
        // TODO: implement is running method.
        return m_window->IsRunning();
    }

    RealityApplication::RealityApplication(const std::string &title, int width, int height) {
        Timer::Init();
        m_window = new Window(title, width, height);
        m_renderer = new Renderer(RenderAPI::Direct3D12, m_window);
    }

    void RealityApplication::Update() {
        m_window->Run();
        Timer::Update();
        m_renderer->Clear();
        m_renderer->RenderStatsUI(Timer::GetFPS(),Timer::GetFrameTimeMS(), m_renderer->GetVSync());
        m_renderer->ProcessStatsUIEvents(m_window->SDL_GetEvent());
    }

    void RealityApplication::Frame() const {
        m_renderer->Frame();
    }

    void RealityApplication::Shutdown() {
        m_renderer->~Renderer();
    }
}
