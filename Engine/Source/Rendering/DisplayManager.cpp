#include "DisplayManager.h"
#include <stdexcept>
#include <Platform/Window.h>

namespace Reality {
    // Initialize the static member
    std::vector<DisplayMode> DisplayManager::modes;

    void DisplayManager::Initialize() {
        if (const int displayCount = SDL_GetNumVideoDisplays(); displayCount < 1) {
            throw std::runtime_error("No displays detected");
        }

        modes.clear();  // Clear existing modes

        int modeCount = SDL_GetNumDisplayModes(0);

        for (int i = 0; i < modeCount; ++i) {
            SDL_DisplayMode mode;
            if (SDL_GetDisplayMode(0, i, &mode)) {
                throw std::runtime_error(SDL_GetError());
            }
            modes.push_back({
                mode.w,
                mode.h,
                mode.refresh_rate,
                mode.format
            });
        }
    }

    const std::vector<DisplayMode>& DisplayManager::GetAvailableModes(int displayIndex) {
        return modes;
    }

    DisplayMode DisplayManager::GetCurrentMode(int displayIndex) {
        SDL_DisplayMode mode;

        if (SDL_GetCurrentDisplayMode(displayIndex, &mode)) {
            throw std::runtime_error(SDL_GetError());
        }
        return { mode.w, mode.h, mode.refresh_rate, mode.format };
    }

    DisplayMode DisplayManager::GetDesktopMode(int displayIndex) {
        SDL_DisplayMode mode;
        if (SDL_GetDesktopDisplayMode(displayIndex, &mode)) {
            throw std::runtime_error(SDL_GetError());
        }
        return { mode.w, mode.h, mode.refresh_rate, mode.format };
    }

    int DisplayManager::FindDisplayMode(const int width, const int height, const int refreshRate) {
        for (int i = 0; i < modes.size(); ++i) {
            if (modes[i].width == width && modes[i].height == height && modes[i].refreshRate == refreshRate) {
                return i;
            }
        }

        return -1;
    }

    bool DisplayManager::ApplyDisplayMode(SDL_Window* window, const DisplayMode& mode, const FullScreenMode &fullScreenMode) {
        SDL_DisplayMode sdlMode;

        SDL_GetCurrentDisplayMode(0, &sdlMode); // Get current to copy format

        sdlMode.w = mode.width;
        sdlMode.h = mode.height;
        sdlMode.refresh_rate = mode.refreshRate;

        if (SDL_SetWindowDisplayMode(window, &sdlMode) != 0) {
            return false;
        }

        auto flags = SDL_GetWindowFlags(window);

        if (fullScreenMode == FullScreenMode::ExclusiveFullScreen) {
            flags = SDL_WINDOW_FULLSCREEN;
        }

        if (fullScreenMode == FullScreenMode::FullScreenWindow) {
            flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
        }

        return SDL_SetWindowFullscreen(window, flags) == 0;
    }

    bool DisplayManager::ApplyDisplayMode(const Reality::Window *window, const DisplayMode &mode, const FullScreenMode &fullScreenMode) {
        return DisplayManager::ApplyDisplayMode(window->m_Window, mode, fullScreenMode);
    }
}
