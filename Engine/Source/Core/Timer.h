#pragma once
#include <chrono>
#include <array>
namespace Reality {
    class Timer {
    public:
        static void Init();

        static void Update();

        // Time in seconds
        static float GetTime();

        // Delta time in seconds
        static float GetDeltaTime();

        // Delta time in milliseconds
        static float GetDeltaTimeMS();

        // Smoothed delta time in seconds
        static float GetSmoothDeltaTime();

        // Smoothed delta time in milliseconds
        static float GetSmoothDeltaTimeMS();

        // Current frame time in milliseconds
        static float GetFrameTimeMS();

        // Smoothed frame time in milliseconds
        static float GetSmoothFrameTimeMS();

        // FPS calculations
        static float GetFPS();
        static uint64_t GetFrameCount();

        // Pause control
        static void SetPaused(bool paused);
        static bool IsPaused();

    private:
        using Clock = std::chrono::high_resolution_clock;
        using TimePoint = std::chrono::time_point<Clock>;
        using Duration = std::chrono::duration<float, std::milli>; // Now using milliseconds as base

        static TimePoint s_StartTime;
        static TimePoint s_LastFrameTime;
        static TimePoint s_CurrentFrameTime;
        static float s_DeltaTime;         // in seconds
        static float s_DeltaTimeMS;       // in milliseconds
        static float s_SmoothDeltaTime;   // in seconds
        static float s_SmoothDeltaTimeMS; // in milliseconds
        static uint64_t s_FrameCount;
        static bool s_Paused;

        // For frame time smoothing
        static constexpr int FRAME_TIME_WINDOW = 60;
        static std::array<float, FRAME_TIME_WINDOW> s_FrameTimeSamples;
        static int s_CurrentSampleIndex;
        static float s_SmoothedFrameTimeMS;
    };
}