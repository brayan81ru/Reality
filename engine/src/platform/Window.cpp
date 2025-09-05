#include "Window.h"
#include <stdexcept>
#include <NativeWindow.h>
#include <SDL_syswm.h>

#include "rendering/Renderer.h"

namespace Reality {
    NativeWindow Window::SDLWindowToNativeWindow(SDL_Window *window) {
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(window, &wmInfo);
        NativeWindow nativeWindow;

        #ifdef _WIN32
        nativeWindow.hWnd = wmInfo.info.win.window;
        #elif __APPLE__
        nativeWindow.hWnd = wmInfo.info.cocoa.window;// macOS impl
        #elif __linux__
        nativeWindow.hWnd = wmInfo.info.x11.window;
        #nativeWindow.pDisplay = wmInfo.info.x11.display;
        #endif

        return nativeWindow;
    }

    Window::Window(const std::string& title, const int width, const int height) {

        SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");

        //SDL_SetHint(SDL_HINT_WINDOWS_DPI_SCALING, "1");

        //SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_SCALING, "1");

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
            throw std::runtime_error(SDL_GetError());
        }

        // Platform-optimized flags
        Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;

        #if defined(__APPLE__)
        flags |= SDL_WINDOW_ALLOW_HIGHDPI;  // macOS Retina support
        #endif

        m_Window = SDL_CreateWindow(
            title.c_str(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height,
            flags
        );

        if (!m_Window) {
            throw std::runtime_error(SDL_GetError());
        }

        m_quit = false;

        m_Initialized = true;
    }

    Window::~Window() {
        if (m_Window) SDL_DestroyWindow(m_Window);
        if (m_Initialized) SDL_Quit();
    }

void Window::Run() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            m_Event = event;

            switch (event.type) {
                case SDL_QUIT:
                    m_quit = true;
                    break;

                case SDL_WINDOWEVENT:
                    // Handle all window events
                    switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED:
                            ///std::cout << "Windows::Resized" << std::endl;
                            if (m_Renderer) {
                                m_Renderer->RecreateSwapChain();
                            }
                            break;

                    case SDL_WINDOWEVENT_MINIMIZED:
                            // Window was minimized
                            //m_minimized = true;
                            std::cout << "Windows::Minimized" << std::endl;
                            break;

                    case SDL_WINDOWEVENT_RESTORED:
                            // Window was restored (un-minimized)
                            std::cout << "Windows::Restored" << std::endl;
                            break;

                    case SDL_WINDOWEVENT_MAXIMIZED:
                            // Window was maximized
                            std::cout << "Windows::Maximized" << std::endl;
                            break;

                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                            // Window gained focus
                            std::cout << "Windows::GotFocus" << std::endl;
                            break;

                    case SDL_WINDOWEVENT_FOCUS_LOST:
                            // Window lost focus
                            std::cout << "Windows::LostFocus" << std::endl;
                            break;
                        default: break;
                    }
                    break;

                    // Add other event types as needed
                case SDL_KEYDOWN:
                    // Handle key press
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    // Handle mouse button press
                    break;
                default: break;
            }
        }
    }

    bool Window::IsRunning() const {
        return !m_quit;
    }

    const SDL_Event *Window::SDL_GetEvent() const {
        return &m_Event;
    }

    void Window::SetRenderer(Renderer *renderer) {
        m_Renderer = renderer;
    }
}
