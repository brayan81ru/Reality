#pragma once
#include <string>
#include <fstream>
#include <chrono>
#include <mutex>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace Reality {
    enum class LogLevel {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Fatal
    };

    class Log {
    public:
        static Log& GetInstance();

        // Delete copy constructor and assignment operator
        Log(const Log&) = delete;
        Log& operator=(const Log&) = delete;

        // Core logging methods with string input
        void LogMessage(LogLevel level, const std::string& message);

        // Convenience methods for different log levels (string version)
        void Trace(const std::string& message);
        void Debug(const std::string& message);
        void Info(const std::string& message);
        void Warning(const std::string& message);
        void Error(const std::string& message);
        void Fatal(const std::string& message);

        // Convenience methods for different log levels (formatted version)
        template<typename... Args>
        void Trace(const char* format, Args&&... args);

        template<typename... Args>
        void Debug(const char* format, Args&&... args);

        template<typename... Args>
        void Info(const char* format, Args&&... args);

        template<typename... Args>
        void Warning(const char* format, Args&&... args);

        template<typename... Args>
        void Error(const char* format, Args&&... args);

        template<typename... Args>
        void Fatal(const char* format, Args&&... args);

        // Configuration methods
        void SetLevel(LogLevel level);
        void EnableConsoleOutput(bool enabled);
        void EnableFileOutput(bool enabled);
        void SetLogFile(const std::string& filename);
        void EnableColors(bool enabled);

        // Helper methods
        [[nodiscard]] static std::string GetLevelString(LogLevel level);
        [[nodiscard]] static std::string GetTimestamp();

    private:
        Log();
        ~Log();

        // Platform-specific initialization
        void InitializeConsole();

        // Output methods
        void WriteToConsole(LogLevel level, const std::string& message) const;
        void WriteToFile(LogLevel level, const std::string& message);

        // Color handling
        void SetConsoleColor(LogLevel level) const;
        void ResetConsoleColor() const;

        // Formatting helper
        static std::string FormatString(const char* format, va_list args);

        // Template formatting helper
        template<typename... Args>
        static std::string FormatString(const char* format, Args&&... args);

        // Member variables
        LogLevel m_currentLevel;
        bool m_consoleEnabled;
        bool m_fileEnabled;
        bool m_colorsEnabled;
        std::string m_logFilename;
        std::ofstream m_logFile;
        std::mutex m_mutex;

#ifdef _WIN32
        HANDLE m_consoleHandle{};
        WORD m_defaultConsoleAttributes{};
#endif
    };

    // Template definitions must be in the header
    template<typename... Args>
    std::string Log::FormatString(const char* format, Args&&... args) {
        // Calculate required buffer size
        std::string::size_type size = snprintf(nullptr, 0, format, args...);
        if (size <= 0) {
            return "";
        }

        // Allocate buffer with space for null terminator
        std::vector<char> buffer(size + 1);

        // Format the string
        snprintf(buffer.data(), buffer.size(), format, args...);

        return {buffer.data(), size};
    }

    template<typename... Args>
    void Log::Trace(const char* format, Args&&... args) {
        const std::string message = FormatString(format, std::forward<Args>(args)...);
        LogMessage(LogLevel::Trace, message);
    }

    template<typename... Args>
    void Log::Debug(const char* format, Args&&... args) {
        const std::string message = FormatString(format, std::forward<Args>(args)...);
        LogMessage(LogLevel::Debug, message);
    }

    template<typename... Args>
    void Log::Info(const char* format, Args&&... args) {
        const std::string message = FormatString(format, std::forward<Args>(args)...);
        LogMessage(LogLevel::Info, message);
    }

    template<typename... Args>
    void Log::Warning(const char* format, Args&&... args) {
        std::string message = FormatString(format, std::forward<Args>(args)...);
        LogMessage(LogLevel::Warning, message);
    }

    template<typename... Args>
    void Log::Error(const char* format, Args&&... args) {
        const std::string message = FormatString(format, std::forward<Args>(args)...);
        LogMessage(LogLevel::Error, message);
    }

    template<typename... Args>
    void Log::Fatal(const char* format, Args&&... args) {
        const std::string message = FormatString(format, std::forward<Args>(args)...);
        LogMessage(LogLevel::Fatal, message);
    }

    // Convenience macros for logging
    #define RLOG_TRACE(...) Reality::Log::GetInstance().Trace(__VA_ARGS__)
    #define RLOG_DEBUG(...) Reality::Log::GetInstance().Debug(__VA_ARGS__)
    #define RLOG_INFO(...) Reality::Log::GetInstance().Info(__VA_ARGS__)
    #define RLOG_WARNING(...) Reality::Log::GetInstance().Warning(__VA_ARGS__)
    #define RLOG_ERROR(...) Reality::Log::GetInstance().Error(__VA_ARGS__)
    #define RLOG_FATAL(...) Reality::Log::GetInstance().Fatal(__VA_ARGS__)
}