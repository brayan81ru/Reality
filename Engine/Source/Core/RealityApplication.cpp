#include "RealityApplication.h"
#include <Rendering/Renderer.h>
#include <Platform/DisplayManager.h>
#include <Core/Timer.h>
#include "Log.h"
#include "Platform/RealityWindow.h"

namespace Reality {
    bool RealityApplication::IsRunning() const {
        return !m_NativeWindow->ShouldClose();
    }

    void RealityApplication::Initialize(const std::string &title, const int width, const int height) {
        Timer::Init();

        m_NativeWindow = new RealityWindow(title,width,height);

        m_renderer = &Renderer::GetInstance();

        m_renderer->Initialize(RenderAPI::Direct3D12, m_NativeWindow);

        // Set event callback for resize events
        m_NativeWindow->SetEventCallback([](const int newWidth, const int newHeight) {
            std::cout << "Window resized to: " << newWidth << "x" << newHeight << std::endl;
            Renderer::GetInstance().WindowResize(newWidth, newHeight);
        });

        m_NativeWindow->Show();
    }

    void RealityApplication::Update() const {
        m_NativeWindow->ProcessMessages();
        Timer::Update();
        m_renderer->Clear();
        m_renderer->RenderStatsUI(Timer::GetFPS(),Timer::GetFrameTimeMS(), m_renderer->GetVSync());
    }

    void RealityApplication::Frame() const {
        m_renderer->Frame();
    }

    void RealityApplication::Shutdown() const {
        m_renderer->~Renderer();
    }
}
