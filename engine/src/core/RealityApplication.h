#pragma once
#include <string>
#include "core/Timer.h"
#include "platform/Window.h"
#include "rendering/Renderer.h"

namespace Reality {

    class RealityApplication {
        Window* m_window;
        Renderer* m_renderer;
    public:
        [[nodiscard]] bool IsRunning() const;

        RealityApplication() = default;

        void Initialize(const std::string& title, int width, int height);

        void Update() const;

        void Frame() const;

        void Shutdown() const;

        [[nodiscard]] Renderer* GetRenderer() const {return m_renderer;}

        [[nodiscard]] Window* GetWindow() const {return m_window;}
    };

}




