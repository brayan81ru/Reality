#include "Log.h"
#include <iostream>
#include <ctime>

namespace Reality {
    Log& Log::GetInstance() {
        static Log instance;
        return instance;
    }

    Log::Log()
        : m_currentLevel(LogLevel::Info)
        , m_consoleEnabled(true)
        , m_fileEnabled(false)
        , m_colorsEnabled(true)
        , m_logFilename("engine.log")
    {
        InitializeConsole();
    }

    Log::~Log() {
        if (m_logFile.is_open()) {
            m_logFile.close();
        }
    }

    void Log::InitializeConsole() {
#ifdef _WIN32
        // Enable ANSI color support on Windows 10+
        m_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (m_consoleHandle != INVALID_HANDLE_VALUE) {
            DWORD mode = 0;
            GetConsoleMode(m_consoleHandle, &mode);
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(m_consoleHandle, mode);

            // Get default console attributes
            CONSOLE_SCREEN_BUFFER_INFO info;
            GetConsoleScreenBufferInfo(m_consoleHandle, &info);
            m_defaultConsoleAttributes = info.wAttributes;
        }
#else
        // For non-Windows systems, ANSI colors are typically supported by default
#endif
    }

    void Log::SetLevel(const LogLevel level) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_currentLevel = level;
    }

    void Log::EnableConsoleOutput(const bool enabled) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_consoleEnabled = enabled;
    }

    void Log::EnableFileOutput(const bool enabled) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_fileEnabled = enabled;

        if (enabled && !m_logFile.is_open()) {
            m_logFile.open(m_logFilename, std::ios::out | std::ios::app);
        } else if (!enabled && m_logFile.is_open()) {
            m_logFile.close();
        }
    }

    void Log::SetLogFile(const std::string& filename) {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_logFile.is_open()) {
            m_logFile.close();
        }

        m_logFilename = filename;

        if (m_fileEnabled) {
            m_logFile.open(m_logFilename, std::ios::out | std::ios::app);
        }
    }

    void Log::EnableColors(const bool enabled) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_colorsEnabled = enabled;
    }

    void Log::LogMessage(const LogLevel level, const std::string& message) {
        if (level < m_currentLevel) {
            return; // Skip messages below current log level
        }

        std::lock_guard<std::mutex> lock(m_mutex);

        // Format the message with timestamp and log level
        std::ostringstream formattedMessage;
        formattedMessage << "[" << GetTimestamp() << "] [" << GetLevelString(level) << "] " << message;

        if (m_consoleEnabled) {
            WriteToConsole(level, formattedMessage.str());
        }

        if (m_fileEnabled) {
            // Ensure file is open before writing
            if (!m_logFile.is_open()) {
                m_logFile.open(m_logFilename, std::ios::out | std::ios::app);
            }

            if (m_logFile.is_open()) {
                WriteToFile(level, formattedMessage.str());
            }
        }
    }

    // String versions of logging methods
    void Log::Trace(const std::string& message) {
        LogMessage(LogLevel::Trace, message);
    }

    void Log::Debug(const std::string& message) {
        LogMessage(LogLevel::Debug, message);
    }

    void Log::Info(const std::string& message) {
        LogMessage(LogLevel::Info, message);
    }

    void Log::Warning(const std::string& message) {
        LogMessage(LogLevel::Warning, message);
    }

    void Log::Error(const std::string& message) {
        LogMessage(LogLevel::Error, message);
    }

    void Log::Fatal(const std::string& message) {
        LogMessage(LogLevel::Fatal, message);
    }

    std::string Log::GetLevelString(const LogLevel level) {
        switch (level) {
            case LogLevel::Trace:   return "TRACE";
            case LogLevel::Debug:   return "DEBUG";
            case LogLevel::Info:    return "INFO";
            case LogLevel::Warning: return "WARNING";
            case LogLevel::Error:   return "ERROR";
            case LogLevel::Fatal:   return "FATAL";
            default:                return "UNKNOWN";
        }
    }

    std::string Log::GetTimestamp() {
        const auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::ostringstream ss;

        // Platform-safe localtime conversion
        std::tm tm_info{};

#ifdef _WIN32
        // Use localtime_s for Windows
        localtime_s(&tm_info, &time_t);
#else
        // Use localtime_r for POSIX systems
        localtime_r(&time_t, &tm_info);
#endif

        ss << std::put_time(&tm_info, "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();

        return ss.str();
    }

    void Log::WriteToConsole(const LogLevel level, const std::string& message) const {
        if (m_colorsEnabled) {
            SetConsoleColor(level);
        }

        std::cout << message << std::endl;

        if (m_colorsEnabled) {
            ResetConsoleColor();
        }
    }

    void Log::WriteToFile(LogLevel level, const std::string& message) {
        // Double-check that the file is open
        if (!m_logFile.is_open()) {
            return;
        }

        m_logFile << message << std::endl;
        m_logFile.flush(); // Ensure immediate write to file
    }

    void Log::SetConsoleColor(LogLevel level) const {
#ifdef _WIN32
        if (m_consoleHandle == INVALID_HANDLE_VALUE) return;

        WORD color;
        switch (level) {
            case LogLevel::Trace:   color = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break; // Cyan
            case LogLevel::Debug:   color = FOREGROUND_BLUE | FOREGROUND_INTENSITY; break; // Light Blue
            case LogLevel::Info:    color = FOREGROUND_GREEN | FOREGROUND_INTENSITY; break; // Light Green
            case LogLevel::Warning: color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break; // Yellow
            case LogLevel::Error:   color = FOREGROUND_RED | FOREGROUND_INTENSITY; break; // Light Red
            case LogLevel::Fatal:   color = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY; break; // Magenta
            default:                color = m_defaultConsoleAttributes; break;
        }

        SetConsoleTextAttribute(m_consoleHandle, color);
#else
        // ANSI escape codes for non-Windows systems
        const char* colorCode;
        switch (level) {
            case LogLevel::Trace:   colorCode = "\033[36m"; break; // Cyan
            case LogLevel::Debug:   colorCode = "\033[94m"; break; // Light Blue
            case LogLevel::Info:    colorCode = "\033[92m"; break; // Light Green
            case LogLevel::Warning: colorCode = "\033[93m"; break; // Yellow
            case LogLevel::Error:   colorCode = "\033[91m"; break; // Light Red
            case LogLevel::Fatal:   colorCode = "\033[95m"; break; // Magenta
            default:                colorCode = "\033[0m"; break;  // Reset
        }

        std::cout << colorCode;
#endif
    }

    void Log::ResetConsoleColor() const {
#ifdef _WIN32
        if (m_consoleHandle != INVALID_HANDLE_VALUE) {
            SetConsoleTextAttribute(m_consoleHandle, m_defaultConsoleAttributes);
        }
#else
        std::cout << "\033[0m"; // Reset ANSI color
#endif
    }

    std::string Log::FormatString(const char *format, va_list args) {
        // Create a copy of va_list in case we need to retry
        va_list args_copy;
        va_copy(args_copy, args);

        // Calculate required buffer size
        std::string::size_type size = vsnprintf(nullptr, 0, format, args_copy);
        va_end(args_copy);

        if (size <= 0) {
            return "";
        }

        // Allocate buffer with space for null terminator
        std::vector<char> buffer(size + 1);

        // Format the string
        vsnprintf(buffer.data(), buffer.size(), format, args);

        return {buffer.data(), size};
    }
}