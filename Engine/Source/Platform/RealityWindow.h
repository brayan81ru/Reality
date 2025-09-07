#pragma once

#include <string>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <objc/objc.h>
#elif defined(__linux__)
#include <X11/Xlib.h>
#endif
namespace Reality {
    class RealityWindow {
    public:
        using EventCallback = std::function<void(int, int)>;

        RealityWindow(const std::string& title, int width, int height);
        ~RealityWindow();

        // Platform-independent methods
        void Show();
        void ProcessMessages();
        bool ShouldClose() const;
        void SetEventCallback(EventCallback callback);

        // Platform-specific getters
#ifdef _WIN32
        HWND GetNativeHandle() const { return m_hwnd; }
#elif defined(__APPLE__)
        id GetNativeHandle() const { return m_window; }
#elif defined(__linux__)
        Window GetNativeHandle() const { return m_window; }
#endif

    private:
        // Platform-specific implementations
        void Initialize();
        void Shutdown();

        // Platform-specific members
#ifdef _WIN32
        HWND m_hwnd = nullptr;
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#elif defined(__APPLE__)
        id m_window = nullptr;
        id m_delegate = nullptr;
        id m_view = nullptr;
        void SetupMacWindow();
        static void OnWindowClose(id self, SEL _cmd, id sender);
        static void OnWindowResize(id self, SEL _cmd, id sender);
#elif defined(__linux__)
        Display* m_display = nullptr;
        Window m_window = 0;
        Atom m_wmDeleteMessage;
        void SetupLinuxWindow();
#endif

        // Common members
        std::string m_title;
        int m_width;
        int m_height;
        bool m_shouldClose = false;
        EventCallback m_eventCallback;
    };
}