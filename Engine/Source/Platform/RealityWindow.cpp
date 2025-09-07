#include "RealityWindow.h"
#include <iostream>

#ifdef _WIN32
#include <windowsx.h>
#elif defined(__APPLE__)
#include <objc/runtime.h>
#include <objc/message.h>
#include <CoreGraphics/CoreGraphics.h>
#elif defined(__linux__)
#include <X11/Xutil.h>
#endif

namespace Reality {
    RealityWindow::RealityWindow(const std::string& title, int width, int height)
        : m_title(title), m_width(width), m_height(height) {
        Initialize();
    }

    RealityWindow::~RealityWindow() {
        Shutdown();
    }

    void RealityWindow::Show() {
#ifdef _WIN32
        ShowWindow(m_hwnd, SW_SHOW);
        UpdateWindow(m_hwnd);
#elif defined(__APPLE__)
        // macOS window is shown by default
        (void)m_window; // Suppress unused warning
#elif defined(__linux__)
        XMapWindow(m_display, m_window);
        XFlush(m_display);
#endif
    }

    void RealityWindow::ProcessMessages() {
#ifdef _WIN32
        MSG msg = {};
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
#elif defined(__APPLE__)
        // macOS uses a run loop, we'll process events in a simplified way
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true);
#elif defined(__linux__)
        XEvent event;
        while (XPending(m_display) > 0) {
            XNextEvent(m_display, &event);
            switch (event.type) {
                case ClientMessage:
                    if (event.xclient.data.l[0] == m_wmDeleteMessage) {
                        m_shouldClose = true;
                    }
                    break;
                case ConfigureNotify:
                    m_width = event.xconfigure.width;
                    m_height = event.xconfigure.height;
                    if (m_eventCallback) {
                        m_eventCallback(m_width, m_height);
                    }
                    break;
                default:
                    break;
            }
        }
#endif
    }

    bool RealityWindow::ShouldClose() const {
        return m_shouldClose;
    }

    void RealityWindow::SetEventCallback(EventCallback callback) {
        m_eventCallback = callback;
    }

    void RealityWindow::Initialize() {
#ifdef _WIN32
        // Register window class
        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = "NativeWindowClass";
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        RegisterClass(&wc);

        // Create window
        m_hwnd = CreateWindowEx(
            0,                              // Optional window styles.
            "NativeWindowClass",            // Window class
            m_title.c_str(),                // Window text
            WS_OVERLAPPEDWINDOW,            // Window style
            CW_USEDEFAULT, CW_USEDEFAULT,   // Position (x, y)
            m_width, m_height,              // Size
            nullptr,                        // Parent window
            nullptr,                        // Menu
            GetModuleHandle(nullptr),       // Instance handle
            this                            // Additional application data
        );

        if (m_hwnd) {
            SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        }
#elif defined(__APPLE__)
        SetupMacWindow();
#elif defined(__linux__)
        SetupLinuxWindow();
#endif
    }

    void RealityWindow::Shutdown() {
#ifdef _WIN32
        if (m_hwnd) {
            DestroyWindow(m_hwnd);
            UnregisterClass("NativeWindowClass", GetModuleHandle(nullptr));
        }
#elif defined(__APPLE__)
        if (m_window) {
            objc_msgSend(m_window, sel_registerName("close"));
            objc_msgSend(m_window, sel_registerName("release"));
        }
        if (m_delegate) {
            objc_msgSend(m_delegate, sel_registerName("release"));
        }
        if (m_view) {
            objc_msgSend(m_view, sel_registerName("release"));
        }
#elif defined(__linux__)
        if (m_display) {
            if (m_window) {
                XDestroyWindow(m_display, m_window);
            }
            XCloseDisplay(m_display);
        }
#endif
    }

    // Windows-specific implementation
#ifdef _WIN32
    LRESULT CALLBACK RealityWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        RealityWindow* window = nullptr;

        if (uMsg == WM_NCCREATE) {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            window = reinterpret_cast<RealityWindow*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        } else {
            window = reinterpret_cast<RealityWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        if (window) {
            switch (uMsg) {
                case WM_DESTROY:
                    window->m_shouldClose = true;
                    return 0;
                case WM_SIZE:
                    window->m_width = LOWORD(lParam);
                    window->m_height = HIWORD(lParam);
                    if (window->m_eventCallback) {
                        window->m_eventCallback(window->m_width, window->m_height);
                    }
                    return 0;
            }
        }

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
#endif

    // macOS-specific implementation
#ifdef __APPLE__
    void NativeWindow::SetupMacWindow() {
        // Create NSAutoreleasePool
        id pool = objc_msgSend((id)objc_getClass("NSAutoreleasePool"), sel_registerName("new"));

        // Get the shared application instance
        id app = objc_msgSend((id)objc_getClass("NSApplication"), sel_registerName("sharedApplication"));

        // Create the window
        id windowRect = objc_msgSend((id)objc_getClass("NSRect"), sel_registerName("rectWithOrigin:size:"),
                                     objc_msgSend((id)objc_getClass("NSPoint"), sel_registerName("pointWithX:y:"), 100.0, 100.0),
                                     objc_msgSend((id)objc_getClass("NSSize"), sel_registerName("sizeWithWidth:height:"), (double)m_width, (double)m_height));

        m_window = objc_msgSend((id)objc_getClass("NSWindow"), sel_registerName("alloc"));
        m_window = objc_msgSend(m_window, sel_registerName("initWithContentRect:styleMask:backing:defer:"),
                                windowRect,
                                15, // NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
                                2,  // NSBackingStoreBuffered
                                false);

        // Set window title
        id titleStr = objc_msgSend((id)objc_getClass("NSString"), sel_registerName("stringWithUTF8String:"), m_title.c_str());
        objc_msgSend(m_window, sel_registerName("setTitle:"), titleStr);

        // Create delegate
        id delegateClass = objc_allocateClassPair((id)objc_getClass("NSObject"), "WindowDelegate", 0);
        class_addMethod(delegateClass, sel_registerName("windowShouldClose:"), (IMP)OnWindowClose, "c@:@");
        class_addMethod(delegateClass, sel_registerName("windowDidResize:"), (IMP)OnWindowResize, "v@:@");
        objc_registerClassPair(delegateClass);

        m_delegate = objc_msgSend((id)delegateClass, sel_registerName("new"));
        objc_msgSend(m_delegate, sel_registerName("setWindow:"), (id)this);
        objc_msgSend(m_window, sel_registerName("setDelegate:"), m_delegate);

        // Create view
        m_view = objc_msgSend((id)objc_getClass("NSView"), sel_registerName("alloc"));
        m_view = objc_msgSend(m_view, sel_registerName("initWithFrame:"), windowRect);
        objc_msgSend(m_window, sel_registerName("setContentView:"), m_view);

        // Center window
        objc_msgSend(m_window, sel_registerName("center"));

        // Release pool
        objc_msgSend(pool, sel_registerName("release"));
    }

    void NativeWindow::OnWindowClose(id self, SEL _cmd, id sender) {
        NativeWindow* window = (NativeWindow*)objc_msgSend(self, sel_registerName("window"));
        if (window) {
            window->m_shouldClose = true;
        }
    }

    void NativeWindow::OnWindowResize(id self, SEL _cmd, id sender) {
        NativeWindow* window = (NativeWindow*)objc_msgSend(self, sel_registerName("window"));
        if (window) {
            id view = objc_msgSend(window->m_window, sel_registerName("contentView"));
            id frame = objc_msgSend(view, sel_registerName("frame"));
            id size = objc_msgSend(frame, sel_registerName("size"));

            window->m_width = (int)objc_msgSend(size, sel_registerName("width"));
            window->m_height = (int)objc_msgSend(size, sel_registerName("height"));

            if (window->m_eventCallback) {
                window->m_eventCallback(window->m_width, window->m_height);
            }
        }
    }
#endif

    // Linux-specific implementation
#ifdef __linux__
    void NativeWindow::SetupLinuxWindow() {
        m_display = XOpenDisplay(nullptr);
        if (!m_display) {
            std::cerr << "Failed to open X display" << std::endl;
            return;
        }

        int screen = DefaultScreen(m_display);
        Window root = RootWindow(m_display, screen);

        // Create window
        m_window = XCreateSimpleWindow(
            m_display, root,
            100, 100, m_width, m_height,
            1, BlackPixel(m_display, screen), WhitePixel(m_display, screen)
        );

        // Set window title
        XStoreName(m_display, m_window, m_title.c_str());

        // Select input events
        XSelectInput(m_display, m_window, StructureNotifyMask | ExposureMask);

        // Set up window manager delete message
        m_wmDeleteMessage = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(m_display, m_window, &m_wmDeleteMessage, 1);
    }
#endif
}