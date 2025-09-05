#pragma once
#include <string>
#include "core/Timer.h"
#include "platform/Window.h"
#include "rendering/Renderer.h"

namespace Reality {

    class RealityApplication {
    private:
        Window* m_window;
        Renderer* m_renderer;
    public:
        bool IsRunning();

        RealityApplication(const std::string& title, int width, int height);

        void Update();

        void Frame() const;

        void Shutdown();

        [[nodiscard]] Renderer* GetRenderer() const {return m_renderer;}

        [[nodiscard]] Window* GetWindow() const {return m_window;}
    };

}




