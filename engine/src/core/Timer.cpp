#include "Timer.h"
#include <algorithm>
namespace Reality {
    // Initialize static members
    Timer::TimePoint Timer::s_StartTime;
    Timer::TimePoint Timer::s_LastFrameTime;
    Timer::TimePoint Timer::s_CurrentFrameTime;
    float Timer::s_DeltaTime = 0.0f;
    float Timer::s_DeltaTimeMS = 0.0f;
    float Timer::s_SmoothDeltaTime = 0.0f;
    float Timer::s_SmoothDeltaTimeMS = 0.0f;
    uint64_t Timer::s_FrameCount = 0;
    bool Timer::s_Paused = false;
    std::array<float, Timer::FRAME_TIME_WINDOW> Timer::s_FrameTimeSamples;
    int Timer::s_CurrentSampleIndex = 0;
    float Timer::s_SmoothedFrameTimeMS = 16.666f; // Initialize to ~60FPS

    void Timer::Init() {
        s_StartTime = Clock::now();
        s_LastFrameTime = s_StartTime;
        s_CurrentFrameTime = s_StartTime;
        std::ranges::fill(s_FrameTimeSamples, 16.666f);
    }

    void Timer::Update() {
        if (s_Paused) {
            s_DeltaTime = 0.0f;
            s_DeltaTimeMS = 0.0f;
            return;
        }

        s_LastFrameTime = s_CurrentFrameTime;
        s_CurrentFrameTime = Clock::now();

        // Calculate raw delta time in milliseconds
        Duration delta = s_CurrentFrameTime - s_LastFrameTime;
        s_DeltaTimeMS = delta.count();
        s_DeltaTime = s_DeltaTimeMS * 0.001f; // Convert to seconds

        // Clamp to avoid extreme values (e.g., during debugging)
        constexpr float MAX_DELTA_MS = 100.0f; // 100ms max frame time
        s_DeltaTimeMS = std::min(s_DeltaTimeMS, MAX_DELTA_MS);
        s_DeltaTime = std::min(s_DeltaTime, MAX_DELTA_MS * 0.001f);

        // Update smooth delta time (weighted average)
        constexpr float smoothFactor = 0.2f;
        s_SmoothDeltaTimeMS = s_SmoothDeltaTimeMS * (1.0f - smoothFactor) + s_DeltaTimeMS * smoothFactor;
        s_SmoothDeltaTime = s_SmoothDeltaTimeMS * 0.001f;

        // Update frame time samples for smoothing
        s_FrameTimeSamples[s_CurrentSampleIndex] = s_DeltaTimeMS;
        s_CurrentSampleIndex = (s_CurrentSampleIndex + 1) % FRAME_TIME_WINDOW;

        // Calculate smoothed frame time (average of last N frames)
        float total = 0.0f;
        for (float sample : s_FrameTimeSamples) {
            total += sample;
        }
        s_SmoothedFrameTimeMS = total / FRAME_TIME_WINDOW;

        s_FrameCount++;
    }

    float Timer::GetTime() {
        Duration timeSinceStart = s_CurrentFrameTime - s_StartTime;
        return timeSinceStart.count() * 0.001f; // Return in seconds
    }

    float Timer::GetDeltaTime() { return s_Paused ? 0.0f : s_DeltaTime; }
    float Timer::GetDeltaTimeMS() { return s_Paused ? 0.0f : s_DeltaTimeMS; }
    float Timer::GetSmoothDeltaTime() { return s_Paused ? 0.0f : s_SmoothDeltaTime; }
    float Timer::GetSmoothDeltaTimeMS() { return s_Paused ? 0.0f : s_SmoothDeltaTimeMS; }

    float Timer::GetFrameTimeMS() {
        return s_Paused ? 0.0f : s_DeltaTimeMS;
    }

    float Timer::GetSmoothFrameTimeMS() {
        return s_Paused ? 0.0f : s_SmoothedFrameTimeMS;
    }

    float Timer::GetFPS() {
        if (s_Paused || s_SmoothedFrameTimeMS == 0.0f) return 0.0f;
        return 1000.0f / s_SmoothedFrameTimeMS;
    }

    uint64_t Timer::GetFrameCount() { return s_FrameCount; }
    void Timer::SetPaused(bool paused) { s_Paused = paused; }
    bool Timer::IsPaused() { return s_Paused; }
}