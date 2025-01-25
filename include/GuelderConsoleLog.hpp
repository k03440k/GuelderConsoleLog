//The main file, which must be included in order to work
#pragma once

#include "GuelderConsoleLogMacroses.hpp"

#include <Windows.h>

#include <iostream>
#include <mutex>
#include <string_view>
#include <string>
#include <sstream>
#include <chrono>
#include <type_traits>
#include <functional>

template<typename T>
concept HasOutputOperator = requires(std::stringstream & os, const T & t) { os << t; };

//enums of color attributes
namespace GuelderConsoleLog
{
    enum class ConsoleForegroundColor : WORD
    {
        Black = 0,
        Blue = FOREGROUND_BLUE,
        Green = FOREGROUND_GREEN,
        Cyan = FOREGROUND_GREEN | FOREGROUND_BLUE,
        Red = FOREGROUND_RED,
        Magenta = FOREGROUND_RED | FOREGROUND_BLUE,
        Yellow = FOREGROUND_RED | FOREGROUND_GREEN,
        White = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        Gray = FOREGROUND_INTENSITY,
        BrightBlue = FOREGROUND_BLUE | FOREGROUND_INTENSITY,
        BrightGreen = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        BrightCyan = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
        BrightRed = FOREGROUND_RED | FOREGROUND_INTENSITY,
        BrightMagenta = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
        BrightYellow = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        BrightWhite = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
    };

    enum class ConsoleBackgroundColor : WORD
    {
        Black = 0,
        Blue = BACKGROUND_BLUE,
        Green = BACKGROUND_GREEN,
        Cyan = BACKGROUND_GREEN | BACKGROUND_BLUE,
        Red = BACKGROUND_RED,
        Magenta = BACKGROUND_RED | BACKGROUND_BLUE,
        Yellow = BACKGROUND_RED | BACKGROUND_GREEN,
        White = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE
    };
}

namespace GuelderConsoleLog
{
    template<typename... Attributes>
    concept ConsoleColorAttributes = ((std::is_same_v<Attributes, ConsoleForegroundColor> || ...)
        || (std::is_same_v<Attributes, ConsoleBackgroundColor> || ...));

    enum class LogLevel : uint8_t
    {
        Info,
        Warning,
        Error,
        All = Info | Warning | Error
    };

    //TODO: ACCOMPLISH A NEW TEMPLATE PARAM
    /// <typeparam name="LoggingLevels">Which levels to handle</typeparam>
    /// <typeparam name="_enable">Whether it would work</typeparam>
    template<LogLevel LoggingLevels, bool _enable>
    struct LoggingCategory {};//TODO: add colors support to this struct

    template<LogLevel LoggingLevels>
    struct LoggingCategory<LoggingLevels, false>
    {
        constexpr LoggingCategory(const std::string_view& name, const bool writeTime = false) {}

        static constexpr LogLevel supportedLoggingLevels = LoggingLevels;
        static constexpr bool enable = false;
    };

    template<LogLevel LoggingLevels>
    struct LoggingCategory<LoggingLevels, true>
    {
        constexpr LoggingCategory(const std::string_view& name, const bool writeTime = false)
            : name(name), writeTime(writeTime) {}
        virtual ~LoggingCategory() = default;

        constexpr bool CanSupportLogLevel(const LogLevel& level) const
        {
            if(supportedLoggingLevels == LogLevel::All)
                return true;
            return static_cast<std::underlying_type_t<LogLevel>>(level) &
                static_cast<std::underlying_type_t<LogLevel>>(supportedLoggingLevels);
        }

        static constexpr LogLevel supportedLoggingLevels = LoggingLevels;
        const std::string_view name;
        static constexpr bool enable = true;
        bool writeTime : 1;
    };

    /// <summary>
    /// Note: I use HANDE but i don't do CloseHandle in dtor
    /// </summary>
    class Logger
    {
    public:
        Logger() = delete;
        ~Logger() = delete;

        template<LogLevel LoggingLevels, typename... Args>
        constexpr static void Log(const LoggingCategory<LoggingLevels, true>& category, const LogLevel& level, Args&&... args)
        {
            std::ostringstream oss;
            Format(oss, args...);
            const std::string message = oss.str();
            WriteLog<LoggingLevels>(category, level, message);
        }
        /// <summary>
        /// disabled if enable == false
        /// </summary>
        template<LogLevel LoggingLevels, typename... Args>
        constexpr static void Log(const LoggingCategory<LoggingLevels, false>& category, const LogLevel& level, Args&&... args) {}

        [[noreturn]]
        static void Throw(const std::string_view& message, const char* fileName, const uint32_t& line);

        [[noreturn]]
        static void Throw(const std::string_view& message);
        /// <returns>
        /// formated std::string from different the template pack
        /// </returns>
        template<HasOutputOperator ...Args>
        constexpr static std::string Format(Args&&... message)
        {
            std::ostringstream out;
            (out << ... << message);
            return out.str();
        }

        /// <summary>
        /// if input bool is false, then it will bring throw of runtime_error
        /// </summary>
        static void Assert(const bool& condition, const std::string& message = "", const char* file = __FILE__, const uint32_t& line = __LINE__);

    private:
        static std::mutex logMutex;
        static HANDLE console;

        template<ConsoleColorAttributes... Attributes>
        constexpr static void SetConsoleColorAttributes(Attributes&&... attrs)
        {
            SetConsoleTextAttribute(console, static_cast<WORD>((static_cast<WORD>(attrs) | ...)));
        }

        template<typename... Args, std::enable_if_t<((std::is_same_v<Args, WORD> || ...)), int> = 0>
        constexpr static void SetConsoleColorAttributes(Args&&... attrs)
        {
            SetConsoleTextAttribute(console, (attrs | ...));
        }

        template<HasOutputOperator T>
        constexpr static void Format(std::ostringstream& oss, const T& arg)
        {
            oss << arg;
        }

        template<HasOutputOperator T, typename... Args>
        constexpr static void Format(std::ostringstream& oss, const T& arg, Args&&... args)//TODO: CHANGE Args&&..
        {
            oss << arg;
            Format(oss, args...);
        }

        template<LogLevel LoggingLevels>
        static void WriteLog(const LoggingCategory<LoggingLevels, true>& category, const LogLevel& level,
            const std::string_view& message)
        {
            std::lock_guard lock(logMutex);

            if(category.writeTime)
            {
                const auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                struct tm localTime;
                localtime_s(&localTime, &t);

                std::cout << std::put_time(&localTime, "%H:%M:%S") << ' '; // print timestamp
            }

            std::cout << category.name << ": ";

            using Background = ConsoleBackgroundColor;
            using Text = ConsoleForegroundColor;
            if(level == LogLevel::Info && category.CanSupportLogLevel(LogLevel::Info))
            {
                SetConsoleColorAttributes(Background::Cyan, Text::White);
                std::cout << "[INFO]";
                SetConsoleColorAttributes(Text::Cyan, Background::Black);
            }
            else if(level == LogLevel::Warning && category.CanSupportLogLevel(LogLevel::Warning))
            {
                SetConsoleColorAttributes(Background::Yellow, Text::White);
                std::cout << "[WARNING]";
                SetConsoleColorAttributes(Text::Yellow, Background::Black);
            }
            else if(level == LogLevel::Error && category.CanSupportLogLevel(LogLevel::Error))
            {
                SetConsoleColorAttributes(Background::Red, Text::White);
                std::cerr << "[ERROR]";
                SetConsoleColorAttributes(Text::Red, Background::Black);
            }
            else
                Throw(Format("Logger::WriteLog: invalid logging level or ", category.name,
                    " doesn't support any logging level"), __FILE__, __LINE__);

            std::cout << ": " << message << '\n';

            SetConsoleColorAttributes(Text::White, Background::Black);
        }
    };
}

//the main logging category type
namespace GuelderConsoleLog
{
    GE_DECLARE_LOG_CATEGORY_EXTERN(Core, All, true, false, true);
}

//helper functions
namespace GuelderConsoleLog
{
    /// <summary>
    /// Prints into std::cout as custom type.
    /// Another form of Logger::Log(color, categoryName, ...)
    /// </summary>
    template<LogLevel LoggingLevels, bool enable, typename... Args>
    constexpr void Log(const LoggingCategory<LoggingLevels, enable>& category, const LogLevel& level, Args&&... info)
    {
        Logger::Log<LoggingLevels>(category, level, info...);//TODO: remake custom log with making macro which will declare new struct which will be inherited from IConsoleCategory with method GetName() (see in the screenshot)
    }

    //Added those just because it writes simpler rather than GE_LOG(Core, Infom ...)

    /// <summary>
    /// Prints into std::cout as info.
    /// Another form of Logger::Log(LogLevel::Info, ...)
    /// </summary>
    template<typename... Args>
    constexpr void LogInfo(Args&&... info)
    {
        GE_LOG(Core, Info, info...);
    }

    /// <summary>
    /// Prints into std::cout as the warning.
    /// Another form of Logger::Log(LogLevel::Warning, ...)
    /// </summary>
    template<typename ...Args>
    constexpr void LogWarning(Args&&... info)
    {
        GE_LOG(Core, Warning, info...);
    }

    /// <summary>
    /// Prints into std::cout as the error.
    /// Another form of Logger::Log(LogLevel::Error, ...)
    /// </summary>
    template<typename ...Args>
    constexpr void LogError(Args&&... info)
    {
        GE_LOG(Core, Error, info...);
    }
}