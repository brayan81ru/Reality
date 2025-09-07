#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <dxgi1_6.h>

namespace Reality {
    struct DisplayResolution {
        int width;
        int height;
        int refreshRate; // in Hz
    };

    enum class PixelFormat {
        UNKNOWN,
        RGBA8,        // 8 bits per channel, 32 bits total
        RGBA10_A2,    // 10 bits per RGB channel, 2 bits for alpha, 32 bits total
        RGBA16,       // 16 bits per channel, 64 bits total
        RGBA16F,      // 16-bit float per channel, 64 bits total
        RGBA32F,      // 32-bit float per channel, 128 bits total
        RGB8,         // 8 bits per channel, 24 bits total
        RGB10,        // 10 bits per channel, 30 bits total
        RGB16,        // 16 bits per channel, 48 bits total
        RGB16F,       // 16-bit float per channel, 48 bits total
        RGB32F        // 32-bit float per channel, 96 bits total
    };

    struct DisplayInfo {
        int width;
        int height;
        int refreshRate;
        bool isHDRSupported;
        PixelFormat pixelFormat;
        int colorDepth; // Total bits per pixel
        float maxLuminance; // in nits
        float minLuminance; // in nits
        float maxFullFrameLuminance; // in nits
    };

    class DisplayManager {
    public:
        DisplayManager();
        ~DisplayManager();

        // Get all available display resolutions
        std::vector<DisplayResolution> GetAvailableResolutions() const;

        // Get current display information
        DisplayInfo GetCurrentDisplayInfo() const;

        // Check if the current display supports HDR
        bool IsHDRSupported() const;

        // Get current display resolution
        DisplayResolution GetCurrentResolution() const;

        // Get current pixel format
        PixelFormat GetPixelFormat() const;

        // Convert PixelFormat to string
        static std::string PixelFormatToString(PixelFormat format);

    private:
        // Platform-specific implementations
        void Initialize();
        void Shutdown();

        // Platform-specific pixel format detection
        PixelFormat DetectPixelFormat() const;
        DisplayInfo GetDisplayInfoInternal() const;

        // Platform-specific data
#ifdef _WIN32
        // Windows-specific members
        mutable bool m_dxgiInitialized = false;
        mutable IDXGIFactory6* m_dxgiFactory = nullptr;
        void InitializeDXGI() const;
#elif defined(__APPLE__)
        // macOS-specific members
#elif defined(__linux__)
        void* m_display; // Display* for X11
        mutable bool m_randrInitialized = false;
        mutable int m_randrEventBase = 0;
        mutable int m_randrErrorBase = 0;
        void InitializeRandR() const;
#endif
    };
}
