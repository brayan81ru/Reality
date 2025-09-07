#pragma once
#include <string>

namespace Reality {
    class Renderer;
    class Window;
    class string;
    class DisplayManager;

    class RealityApplication {
        Window* m_window;
        Renderer* m_renderer;
        DisplayManager* m_displayManager;
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




