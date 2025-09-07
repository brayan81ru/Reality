#include "DisplayManager.h"
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <vector>
#include <dxgi1_6.h> // For HDR support and advanced display info
#include <d3d11.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#elif defined(__APPLE__)
#include <CoreGraphics/CoreGraphics.h>
#include <ApplicationServices/ApplicationServices.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#elif defined(__linux__)
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/Xrandr.h>
#include <vector>
#include <string>
#include <fstream>
#include <regex>
#endif

namespace Reality {
    DisplayManager::DisplayManager() {
        Initialize();
    }

    DisplayManager::~DisplayManager() {
        Shutdown();
    }

    void DisplayManager::Initialize() {
#ifdef _WIN32
        // Initialize Windows-specific resources if needed
#elif defined(__APPLE__)
        // Initialize macOS-specific resources
#elif defined(__linux__)
        m_display = XOpenDisplay(nullptr);
        InitializeRandR();
#endif
    }

    void DisplayManager::Shutdown() {
#ifdef _WIN32
        if (m_dxgiFactory) {
            m_dxgiFactory->Release();
        }
#elif defined(__APPLE__)
        // Cleanup macOS resources
#elif defined(__linux__)
        if (m_display) {
            XCloseDisplay(static_cast<Display*>(m_display));
        }
#endif
    }

#ifdef _WIN32
    void DisplayManager::InitializeDXGI() const {
        if (m_dxgiInitialized) return;

        if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)))) {
            std::cerr << "Failed to create DXGI factory" << std::endl;
            return;
        }

        m_dxgiInitialized = true;
    }
#endif

#ifdef __linux__
    void DisplayManager::InitializeRandR() const {
        if (!m_display || m_randrInitialized) return;

        int major, minor;
        if (!XRRQueryVersion(static_cast<Display*>(m_display), &major, &minor)) {
            std::cerr << "XRandR not supported" << std::endl;
            return;
        }

        if (!XRRQueryExtension(static_cast<Display*>(m_display), &m_randrEventBase, &m_randrErrorBase)) {
            std::cerr << "Failed to query XRandR extension" << std::endl;
            return;
        }

        m_randrInitialized = true;
    }
#endif

    std::vector<DisplayResolution> DisplayManager::GetAvailableResolutions() const {
        std::vector<DisplayResolution> resolutions;

#ifdef _WIN32
        DEVMODE devMode = {};
        devMode.dmSize = sizeof(DEVMODE);

        for (int i = 0; EnumDisplaySettings(nullptr, i, &devMode); i++) {
            // Only consider modes with a valid refresh rate
            if (devMode.dmDisplayFrequency > 0) {
                DisplayResolution res = {
                    devMode.dmPelsWidth,
                    devMode.dmPelsHeight,
                    devMode.dmDisplayFrequency
                };
                resolutions.push_back(res);
            }
        }
#elif defined(__APPLE__)
        // On macOS, we can use CGDisplayCopyAllDisplayModes
        CGDirectDisplayID displayID = CGMainDisplayID();
        CFArrayRef modes = CGDisplayCopyAllDisplayModes(displayID, nullptr);
        if (modes) {
            CFIndex count = CFArrayGetCount(modes);
            for (CFIndex i = 0; i < count; i++) {
                CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(modes, i);
                DisplayResolution res = {
                    (int)CGDisplayModeGetWidth(mode),
                    (int)CGDisplayModeGetHeight(mode),
                    (int)CGDisplayModeGetRefreshRate(mode)
                };
                resolutions.push_back(res);
            }
            CFRelease(modes);
        }
#elif defined(__linux__)
        if (!m_display || !m_randrInitialized) {
            return resolutions;
        }

        Display* display = static_cast<Display*>(m_display);
        Window root = RootWindow(display, DefaultScreen(display));
        XRRScreenResources* res = XRRGetScreenResources(display, root);
        if (!res) {
            return resolutions;
        }

        for (int i = 0; i < res->noutput; i++) {
            XRROutputInfo* outputInfo = XRRGetOutputInfo(display, res, res->outputs[i]);
            if (!outputInfo || outputInfo->connection != RR_Connected) {
                if (outputInfo) XRRFreeOutputInfo(outputInfo);
                continue;
            }

            for (int j = 0; j < outputInfo->nmode; j++) {
                RRMode modeID = outputInfo->modes[j];
                XRRModeInfo* modeInfo = nullptr;
                for (int k = 0; k < res->nmode; k++) {
                    if (res->modes[k].id == modeID) {
                        modeInfo = &res->modes[k];
                        break;
                    }
                }

                if (modeInfo) {
                    DisplayResolution res = {
                        (int)modeInfo->width,
                        (int)modeInfo->height,
                        (int)(modeInfo->dotClock / (modeInfo->hTotal * modeInfo->vTotal))
                    };
                    resolutions.push_back(res);
                }
            }
            XRRFreeOutputInfo(outputInfo);
        }
        XRRFreeScreenResources(res);
#endif

        return resolutions;
    }

    PixelFormat DisplayManager::DetectPixelFormat() const {
#ifdef _WIN32
        InitializeDXGI();
        if (!m_dxgiFactory) return PixelFormat::UNKNOWN;

        // Get the primary adapter
        IDXGIAdapter* adapter = nullptr;
        for (UINT i = 0; m_dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
            IDXGIOutput* output = nullptr;
            for (UINT j = 0; adapter->EnumOutputs(j, &output) != DXGI_ERROR_NOT_FOUND; j++) {
                DXGI_OUTPUT_DESC desc;
                if (SUCCEEDED(output->GetDesc(&desc))) {
                    // Check if this is the primary display
                    if (desc.AttachedToDesktop) {
                        IDXGIOutput6* output6 = nullptr;
                        if (SUCCEEDED(output->QueryInterface(IID_PPV_ARGS(&output6)))) {
                            DXGI_OUTPUT_DESC1 desc1;
                            if (SUCCEEDED(output6->GetDesc1(&desc1))) {
                                // Check color space and format
                                if (desc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020) {
                                    // HDR display
                                    if (desc1.BitsPerColor == 10) {
                                        output6->Release();
                                        output->Release();
                                        adapter->Release();
                                        return PixelFormat::RGBA10_A2;
                                    } else if (desc1.BitsPerColor == 16) {
                                        output6->Release();
                                        output->Release();
                                        adapter->Release();
                                        return PixelFormat::RGBA16F;
                                    }
                                } else {
                                    // SDR display
                                    if (desc1.BitsPerColor == 8) {
                                        output6->Release();
                                        output->Release();
                                        adapter->Release();
                                        return PixelFormat::RGBA8;
                                    } else if (desc1.BitsPerColor == 10) {
                                        output6->Release();
                                        output->Release();
                                        adapter->Release();
                                        return PixelFormat::RGBA10_A2;
                                    } else if (desc1.BitsPerColor == 16) {
                                        output6->Release();
                                        output->Release();
                                        adapter->Release();
                                        return PixelFormat::RGBA16;
                                    }
                                }
                            }
                            output6->Release();
                        }
                        break;
                    }
                }
                output->Release();
            }
            adapter->Release();
        }

        // Fallback to desktop settings
        HDC hdc = GetDC(nullptr);
        int bpp = GetDeviceCaps(hdc, BITSPIXEL);
        ReleaseDC(nullptr, hdc);

        if (bpp == 32) return PixelFormat::RGBA8;
        if (bpp == 24) return PixelFormat::RGB8;
        if (bpp == 30) return PixelFormat::RGB10;
        if (bpp == 48) return PixelFormat::RGB16;

        return PixelFormat::UNKNOWN;

#elif defined(__APPLE__)
        CGDirectDisplayID displayID = CGMainDisplayID();

        // Get display mode information
        CGDisplayModeRef mode = CGDisplayCopyDisplayMode(displayID);
        if (!mode) return PixelFormat::UNKNOWN;

        // Get pixel encoding
        CFStringRef pixelEncoding = CGDisplayModeCopyPixelEncoding(mode);
        if (!pixelEncoding) {
            CGDisplayModeRelease(mode);
            return PixelFormat::UNKNOWN;
        }

        PixelFormat format = PixelFormat::UNKNOWN;

        if (CFStringCompare(pixelEncoding, CFSTR(IO32BitDirectPixels), 0) == kCFCompareEqualTo) {
            // Check for HDR support
            id mainScreen = objc_msgSend((id)objc_getClass("NSScreen"), sel_registerName("mainScreen"));
            if (mainScreen) {
                id colorSpace = objc_msgSend(mainScreen, sel_registerName("colorSpace"));
                if (colorSpace) {
                    id localizedName = objc_msgSend(colorSpace, sel_registerName("localizedName"));
                    const char* name = reinterpret_cast<const char*>(objc_msgSend(localizedName, sel_registerName("UTF8String")));
                    if (strstr(name, "Display P3") || strstr(name, "HDR")) {
                        // HDR display
                        format = PixelFormat::RGBA10_A2; // Common for HDR on macOS
                    } else {
                        // SDR display
                        format = PixelFormat::RGBA8;
                    }
                }
            }

            if (format == PixelFormat::UNKNOWN) {
                // Fallback to checking bit depth
                size_t depth = CGDisplayModeGetBitsPerPixel(mode);
                if (depth == 32) format = PixelFormat::RGBA8;
                else if (depth == 64) format = PixelFormat::RGBA16;
            }
        } else if (CFStringCompare(pixelEncoding, CFSTR(IO16BitDirectPixels), 0) == kCFCompareEqualTo) {
            format = PixelFormat::RGBA16;
        } else if (CFStringCompare(pixelEncoding, CFSTR(kIO30BitDirectPixels), 0) == kCFCompareEqualTo) {
            format = PixelFormat::RGB10;
        }

        CFRelease(pixelEncoding);
        CGDisplayModeRelease(mode);

        return format;

#elif defined(__linux__)
        if (!m_display || !m_randrInitialized) return PixelFormat::UNKNOWN;

        Display* display = static_cast<Display*>(m_display);
        Window root = RootWindow(display, DefaultScreen(display));

        // Try to get the visual info for the default visual
        XVisualInfo visualTemplate = {};
        visualTemplate.visualid = XVisualIDFromVisual(DefaultVisual(display, DefaultScreen(display)));
        int numVisuals;
        XVisualInfo* visualInfo = XGetVisualInfo(display, VisualIDMask, &visualTemplate, &numVisuals);

        if (!visualInfo || numVisuals == 0) {
            if (visualInfo) XFree(visualInfo);
            return PixelFormat::UNKNOWN;
        }

        PixelFormat format = PixelFormat::UNKNOWN;

        // Determine format based on visual depth and class
        if (visualInfo->depth == 32 && visualInfo->c_class == TrueColor) {
            // Check for 10-bit color depth
            Atom depthAtom = XInternAtom(display, "_XRANDR_DEPTH", True);
            if (depthAtom != None) {
                // Try to get the actual depth from XRandR
                XRRScreenResources* res = XRRGetScreenResources(display, root);
                if (res) {
                    RROutput primaryOutput = XRRGetOutputPrimary(display, root);
                    if (primaryOutput != None) {
                        XRROutputInfo* outputInfo = XRRGetOutputInfo(display, res, primaryOutput);
                        if (outputInfo) {
                            // Check for HDR support
                            Atom hdrAtom = XInternAtom(display, "HDR_OUTPUT", True);
                            if (hdrAtom != None) {
                                // Check if the output has HDR property
                                for (int i = 0; i < outputInfo->nmode; i++) {
                                    RRMode modeID = outputInfo->modes[i];
                                    for (int j = 0; j < res->nmode; j++) {
                                        if (res->modes[j].id == modeID) {
                                            // Check if this is the current mode
                                            XRRScreenConfiguration* config = XRRGetScreenInfo(display, root);
                                            if (config) {
                                                Rotation rotation;
                                                RRMode currentMode = XRRConfigCurrentConfiguration(config, &rotation);
                                                XRRFreeScreenConfigInfo(config);

                                                if (currentMode == modeID) {
                                                    // Current mode, check for HDR
                                                    format = PixelFormat::RGBA10_A2;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    if (format != PixelFormat::UNKNOWN) break;
                                }
                            }
                            XRRFreeOutputInfo(outputInfo);
                        }
                    }
                    XRRFreeScreenResources(res);
                }
            }

            if (format == PixelFormat::UNKNOWN) {
                // Fallback to 8-bit
                format = PixelFormat::RGBA8;
            }
        } else if (visualInfo->depth == 24 && visualInfo->c_class == TrueColor) {
            format = PixelFormat::RGB8;
        } else if (visualInfo->depth == 30 && visualInfo->c_class == TrueColor) {
            format = PixelFormat::RGB10;
        } else if (visualInfo->depth == 48 && visualInfo->c_class == TrueColor) {
            format = PixelFormat::RGB16;
        }

        XFree(visualInfo);
        return format;
#endif
    }

    DisplayInfo DisplayManager::GetDisplayInfoInternal() const {
        DisplayInfo info = {};

#ifdef _WIN32
        // Get current resolution
        DEVMODE devMode = {};
        devMode.dmSize = sizeof(DEVMODE);
        EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);

        info.width = devMode.dmPelsWidth;
        info.height = devMode.dmPelsHeight;
        info.refreshRate = devMode.dmDisplayFrequency;

        // Check HDR support and get luminance info
        info.isHDRSupported = false;
        info.maxLuminance = 0.0f;
        info.minLuminance = 0.0f;
        info.maxFullFrameLuminance = 0.0f;

        InitializeDXGI();
        if (m_dxgiFactory) {
            IDXGIAdapter* adapter = nullptr;
            for (UINT i = 0; m_dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
                IDXGIOutput* output = nullptr;
                for (UINT j = 0; adapter->EnumOutputs(j, &output) != DXGI_ERROR_NOT_FOUND; j++) {
                    DXGI_OUTPUT_DESC desc;
                    if (SUCCEEDED(output->GetDesc(&desc))) {
                        // Check if this is the primary display
                        if (desc.AttachedToDesktop) {
                            IDXGIOutput6* output6 = nullptr;
                            if (SUCCEEDED(output->QueryInterface(IID_PPV_ARGS(&output6)))) {
                                DXGI_OUTPUT_DESC1 desc1;
                                if (SUCCEEDED(output6->GetDesc1(&desc1))) {
                                    info.isHDRSupported = (desc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
                                    info.maxLuminance = desc1.MaxLuminance;
                                    info.minLuminance = desc1.MinLuminance;
                                    info.maxFullFrameLuminance = desc1.MaxFullFrameLuminance;
                                }
                                output6->Release();
                            }
                            break;
                        }
                    }
                    output->Release();
                }
                adapter->Release();
            }
        }

        // Get pixel format and color depth
        info.pixelFormat = DetectPixelFormat();
        info.colorDepth = devMode.dmBitsPerPel;

#elif defined(__APPLE__)
        // Get current resolution
        CGDirectDisplayID displayID = CGMainDisplayID();
        info.width = (int)CGDisplayPixelsWide(displayID);
        info.height = (int)CGDisplayPixelsHigh(displayID);
        info.refreshRate = (int)CGDisplayModeGetRefreshRate(CGDisplayCopyDisplayMode(displayID));

        // Check HDR support and get luminance info
        info.isHDRSupported = false;
        info.maxLuminance = 0.0f;
        info.minLuminance = 0.0f;
        info.maxFullFrameLuminance = 0.0f;

        // Use IOKit to get display information
        io_service_t displayService = CGDisplayIOServicePort(displayID);
        CFDictionaryRef displayInfoDict = IODisplayCreateInfoDictionary(displayService, kIODisplayOnlyPreferredName);
        if (displayInfoDict) {
            // Get display capabilities
            CFDictionaryRef capabilities = (CFDictionaryRef)CFDictionaryGetValue(displayInfoDict, CFSTR(kDisplayCapabilities));
            if (capabilities) {
                // Check for HDR support
                CFBooleanRef hdrSupported = (CFBooleanRef)CFDictionaryGetValue(capabilities, CFSTR("HDRSupported"));
                if (hdrSupported && CFBooleanGetValue(hdrSupported)) {
                    info.isHDRSupported = true;

                    // Get luminance values
                    CFNumberRef maxLuminance = (CFNumberRef)CFDictionaryGetValue(capabilities, CFSTR("MaxLuminance"));
                    if (maxLuminance) {
                        float value;
                        if (CFNumberGetValue(maxLuminance, kCFNumberFloatType, &value)) {
                            info.maxLuminance = value;
                        }
                    }

                    CFNumberRef minLuminance = (CFNumberRef)CFDictionaryGetValue(capabilities, CFSTR("MinLuminance"));
                    if (minLuminance) {
                        float value;
                        if (CFNumberGetValue(minLuminance, kCFNumberFloatType, &value)) {
                            info.minLuminance = value;
                        }
                    }

                    CFNumberRef maxFullFrameLuminance = (CFNumberRef)CFDictionaryGetValue(capabilities, CFSTR("MaxFullFrameLuminance"));
                    if (maxFullFrameLuminance) {
                        float value;
                        if (CFNumberGetValue(maxFullFrameLuminance, kCFNumberFloatType, &value)) {
                            info.maxFullFrameLuminance = value;
                        }
                    }
                }
            }
            CFRelease(displayInfoDict);
        }

        // Fallback to NSScreen for HDR detection if IOKit didn't work
        if (!info.isHDRSupported) {
            id mainScreen = objc_msgSend((id)objc_getClass("NSScreen"), sel_registerName("mainScreen"));
            if (mainScreen) {
                id colorSpace = objc_msgSend(mainScreen, sel_registerName("colorSpace"));
                if (colorSpace) {
                    id localizedName = objc_msgSend(colorSpace, sel_registerName("localizedName"));
                    const char* name = reinterpret_cast<const char*>(objc_msgSend(localizedName, sel_registerName("UTF8String")));
                    if (strstr(name, "Display P3") || strstr(name, "HDR")) {
                        info.isHDRSupported = true;
                        // Set default luminance values for macOS HDR displays
                        info.maxLuminance = 1000.0f;
                        info.minLuminance = 0.0f;
                        info.maxFullFrameLuminance = 800.0f;
                    }
                }
            }
        }

        // Get pixel format and color depth
        info.pixelFormat = DetectPixelFormat();
        CGDisplayModeRef mode = CGDisplayCopyDisplayMode(displayID);
        if (mode) {
            info.colorDepth = (int)CGDisplayModeGetBitsPerPixel(mode);
            CGDisplayModeRelease(mode);
        }

#elif defined(__linux__)
        if (!m_display || !m_randrInitialized) {
            return info;
        }

        Display* display = static_cast<Display*>(m_display);
        Window root = RootWindow(display, DefaultScreen(display));
        XRRScreenResources* res = XRRGetScreenResources(display, root);
        if (!res) {
            return info;
        }

        // Get current configuration
        XRRScreenConfiguration* config = XRRGetScreenInfo(display, root);
        if (config) {
            Rotation rotation;
            RRMode currentMode = XRRConfigCurrentConfiguration(config, &rotation);
            info.refreshRate = (int)XRRConfigCurrentRate(config);

            // Get the current mode's resolution
            for (int i = 0; i < res->nmode; i++) {
                if (res->modes[i].id == currentMode) {
                    info.width = (int)res->modes[i].width;
                    info.height = (int)res->modes[i].height;
                    break;
                }
            }
            XRRFreeScreenConfigInfo(config);
        }

        // Check HDR support and get luminance info
        info.isHDRSupported = false;
        info.maxLuminance = 0.0f;
        info.minLuminance = 0.0f;
        info.maxFullFrameLuminance = 0.0f;

        // Try to get the primary output
        RROutput primaryOutput = XRRGetOutputPrimary(display, root);
        if (primaryOutput != None) {
            XRROutputInfo* outputInfo = XRRGetOutputInfo(display, res, primaryOutput);
            if (outputInfo) {
                // Check for HDR support
                Atom hdrAtom = XInternAtom(display, "HDR_OUTPUT", True);
                if (hdrAtom != None) {
                    // Check if the output has HDR property
                    for (int i = 0; i < outputInfo->nmode; i++) {
                        RRMode modeID = outputInfo->modes[i];
                        for (int j = 0; j < res->nmode; j++) {
                            if (res->modes[j].id == modeID) {
                                // Check if this is the current mode
                                XRRScreenConfiguration* config = XRRGetScreenInfo(display, root);
                                if (config) {
                                    Rotation rotation;
                                    RRMode currentMode = XRRConfigCurrentConfiguration(config, &rotation);
                                    XRRFreeScreenConfigInfo(config);

                                    if (currentMode == modeID) {
                                        // Current mode, check for HDR
                                        info.isHDRSupported = true;

                                        // Try to get luminance info from EDID
                                        // This is a simplified approach; a more robust solution would parse the EDID data
                                        info.maxLuminance = 1000.0f; // Default value
                                        info.minLuminance = 0.0f;
                                        info.maxFullFrameLuminance = 800.0f;
                                        break;
                                    }
                                }
                            }
                        }
                        if (info.isHDRSupported) break;
                    }
                }
                XRRFreeOutputInfo(outputInfo);
            }
        }

        // Try to get luminance info from the EDID data
        if (info.isHDRSupported) {
            // Try to read EDID data
            std::ifstream edidFile("/sys/class/drm/card0-HDMI-A-1/edid", std::ios::binary);
            if (!edidFile.is_open()) {
                // Try alternative paths
                edidFile.open("/sys/class/drm/card1-HDMI-A-1/edid", std::ios::binary);
            }

            if (edidFile.is_open()) {
                // Read EDID data
                std::vector<uint8_t> edidData((std::istreambuf_iterator<char>(edidFile)),
                                             std::istreambuf_iterator<char>());

                // Parse EDID for luminance info (simplified)
                // A full implementation would parse the entire EDID structure
                if (edidData.size() >= 128) {
                    // Check for HDR static metadata block in CEA extension
                    for (size_t i = 128; i < edidData.size() - 1; i += 128) {
                        if (edidData[i] == 0x02 && edidData[i+1] == 0x03) {
                            // CEA extension block
                            // Look for HDR static metadata data block
                            for (size_t j = 4; j < 124; j++) {
                                if (edidData[i+j] == 0x07 && edidData[i+j+1] >= 3) {
                                    // HDR static metadata data block
                                    // Extract luminance values (simplified)
                                    info.maxLuminance = (float)(edidData[i+j+3] * 50);
                                    info.minLuminance = (float)(edidData[i+j+4] * 0.01);
                                    info.maxFullFrameLuminance = (float)(edidData[i+j+5] * 50);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Get pixel format and color depth
        info.pixelFormat = DetectPixelFormat();

        // Get color depth from visual
        XVisualInfo visualTemplate = {};
        visualTemplate.visualid = XVisualIDFromVisual(DefaultVisual(display, DefaultScreen(display)));
        int numVisuals;
        XVisualInfo* visualInfo = XGetVisualInfo(display, VisualIDMask, &visualTemplate, &numVisuals);

        if (visualInfo && numVisuals > 0) {
            info.colorDepth = visualInfo->depth;
            XFree(visualInfo);
        }

        XRRFreeScreenResources(res);
#endif

        return info;
    }

    DisplayInfo DisplayManager::GetCurrentDisplayInfo() const {
        return GetDisplayInfoInternal();
    }

    bool DisplayManager::IsHDRSupported() const {
        return GetCurrentDisplayInfo().isHDRSupported;
    }

    DisplayResolution DisplayManager::GetCurrentResolution() const {
        DisplayInfo info = GetCurrentDisplayInfo();
        return { info.width, info.height, info.refreshRate };
    }

    PixelFormat DisplayManager::GetPixelFormat() const {
        return GetCurrentDisplayInfo().pixelFormat;
    }

    std::string DisplayManager::PixelFormatToString(PixelFormat format) {
        switch (format) {
            case PixelFormat::RGBA8: return "RGBA8";
            case PixelFormat::RGBA10_A2: return "RGBA10_A2";
            case PixelFormat::RGBA16: return "RGBA16";
            case PixelFormat::RGBA16F: return "RGBA16F";
            case PixelFormat::RGBA32F: return "RGBA32F";
            case PixelFormat::RGB8: return "RGB8";
            case PixelFormat::RGB10: return "RGB10";
            case PixelFormat::RGB16: return "RGB16";
            case PixelFormat::RGB16F: return "RGB16F";
            case PixelFormat::RGB32F: return "RGB32F";
            case PixelFormat::UNKNOWN:
            default:
                return "UNKNOWN";
        }
    }
}