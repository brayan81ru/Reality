#pragma once
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <string>
#include <NativeWindow.h>

namespace Reality {
    class Renderer;

    class Window {

    public:

        static Diligent::NativeWindow SDLWindowToNativeWindow(SDL_Window* window);

        Window(const std::string& title, int width, int height);

        ~Window();

        [[nodiscard]]
        SDL_Window* GetNativeWindow() const { return m_Window; }

        void Run();

        [[nodiscard]]
        bool IsRunning() const;

        SDL_Window* m_Window = nullptr;

        bool m_Initialized = false;

        [[nodiscard]]
        const SDL_Event *SDL_GetEvent() const;

        void SetRenderer(Renderer* renderer);

    private:
        Renderer* m_Renderer;
        bool m_quit = false;
        SDL_Event m_Event;
    };
}
