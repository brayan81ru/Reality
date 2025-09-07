#pragma once
#include <string>

namespace Reality {
    class RealityWindow;
    class Renderer;
    class Window;
    class string;
    class DisplayInfo;

    class RealityApplication {
        RealityWindow* m_NativeWindow;
        Renderer* m_renderer;
        DisplayInfo* m_displayManager;
    public:
        [[nodiscard]] bool IsRunning() const;

        RealityApplication() = default;

        void Initialize(const std::string& title, int width, int height);

        void Update() const;

        void Frame() const;

        void Shutdown() const;

        [[nodiscard]] Renderer* GetRenderer() const {return m_renderer;}

        [[nodiscard]] RealityWindow* GetWindow() const {return m_NativeWindow;}
    };
}




