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
#include <stdexcept>

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
    inline std::wstring StringToWString(const std::string_view& str)
    {
        int neededSize = MultiByteToWideChar(CP_UTF8, 0, str.data(), -1, nullptr, 0);
        std::wstring wString(neededSize, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, str.data(), -1, wString.data(), neededSize);

        return wString;
    }
    inline std::string WStringToString(const std::wstring_view& wstr)
    {
        int neededSize = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), nullptr, 0, nullptr, nullptr);
        std::string string(neededSize, L'\0');
        WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), string.data(), neededSize, nullptr, nullptr);

        return string;
    }
}
namespace GuelderConsoleLog
{
    template<typename T>
    using RawType = std::remove_cv_t<std::remove_reference_t<std::remove_pointer_t<std::remove_extent_t<T>>>>;

    namespace Concepts
    {
        template<typename T, typename U>
        concept SameRawType = std::is_same_v<RawType<T>, RawType<U>>;

        template<typename... Args>               //idk why RawType<Args>, because the same thing is made inside SameRawType, C++ blows my mind
        concept IsThereAtLeastOneWideChar = (SameRawType<typename RawType<Args>::value_type, wchar_t> || ...) || (SameRawType<RawType<Args>, wchar_t> || ...);

        template<typename String, typename... Strings>
        concept IsAtLeastOneSame = (SameRawType<RawType<String>, Strings> || ...);

        template<typename Char, typename T>
        concept HasOutputOperator = (requires(std::basic_ostream<Char>&os, const T & t) { os << t; });

        template<typename T>
        concept STDCout = HasOutputOperator<char, T>;

        template<typename T>
        concept STDWout = HasOutputOperator<wchar_t, T> && !STDCout<T>;

        template<typename T>
        concept STDOut = STDCout<T> || STDWout<T>;

        template<typename... Attributes>
        concept ConsoleColorAttributes = (std::is_same_v<Attributes, ConsoleForegroundColor> || ...) || (std::is_same_v<Attributes, ConsoleBackgroundColor> || ...);
    }
}
namespace GuelderConsoleLog
{
    enum class LogLevel : uint8_t
    {
        Info,
        Warning,
        Error,
        All = Info | Warning | Error
    };

    template<LogLevel LoggingLevels, bool _enable>
    struct LoggingCategory {};

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

    class Logger
    {
    public:
        Logger() = delete;
        ~Logger() = delete;

        template<LogLevel LoggingLevels, Concepts::STDOut... Args>
        constexpr static void Log(const LoggingCategory<LoggingLevels, true>& category, const LogLevel& level, Args&&... args)
        {
            if constexpr(Concepts::IsThereAtLeastOneWideChar<Args...>)
                WriteLog<wchar_t, LoggingLevels>(category, level, Format(std::forward<Args>(args)...));
            else
                WriteLog<char, LoggingLevels>(category, level, Format(std::forward<Args>(args)...));
        }
        /// <summary>
        /// disabled if enable == false
        /// </summary>
        template<LogLevel LoggingLevels, Concepts::STDOut... Args>
        constexpr static void Log(const LoggingCategory<LoggingLevels, false>& category, const LogLevel& level, Args&&... args) {}

        [[noreturn]]
        static void Throw(const std::string_view& message, const char* const fileName, const uint32_t& line)
        {
            throw(std::runtime_error(Format(message, '\n', "file: ", fileName, ", line: ", line)));
        }

        [[noreturn]]
        static void Throw(const std::string_view& message)
        {
            throw std::runtime_error(message.data());
        }

        /// <summary>
        /// if input bool is false, then it will bring throw of runtime_error
        /// </summary>
        static void Assert(const bool& condition, const std::string_view& message = "", const char* file = __FILE__, const uint32_t& line = __LINE__)
        {
            if(!condition)
            {
                Throw(message, file, line);
                __debugbreak();
            }
        }

        template<Concepts::STDOut... Args>
        constexpr static auto Format(Args&&... message)
        {
            if constexpr(Concepts::IsThereAtLeastOneWideChar<Args...>)
            {
                std::wstring _out;
                {
                    std::wstringstream outStr;
                    (outStr << ... << std::forward<Args>(message));
                    _out = outStr.str();
                }
                std::erase(_out, L'\0');
                return _out;
            }
            else
            {
                std::stringstream outStr;
                (outStr << ... << std::forward<Args>(message));
                return outStr.str();
            }
        }

    private:
        struct LoggerHelper
        {
        public:
            LoggerHelper();

            LoggerHelper(const LoggerHelper&) = delete;
            LoggerHelper(LoggerHelper&&) = delete;

            LoggerHelper& operator=(const LoggerHelper&) = delete;
            LoggerHelper& operator=(LoggerHelper&&) = delete;

        private:
            static std::mutex logMutex;
            static HANDLE console;
        };

    private:
        static std::mutex logMutex;
        static HANDLE console;

        //DON'T USE
        static const LoggerHelper loggerHelper;

    private:
        template<Concepts::STDCout T>
        static std::ostream& Out()
        {
            return std::cout;
        }
        template<Concepts::STDWout T>
        static std::wostream& Out()
        {
            return std::wcout;
        }

        template<Concepts::ConsoleColorAttributes... Attributes>
        constexpr static void SetConsoleColorAttributes(Attributes&&... attrs)
        {
            SetConsoleTextAttribute(console, static_cast<WORD>((static_cast<WORD>(attrs) | ...)));
        }
        template<typename... Args, std::enable_if_t<((std::is_same_v<Args, WORD> || ...)), int> = 0>
        constexpr static void SetConsoleColorAttributes(Args&&... attrs)
        {
            SetConsoleTextAttribute(console, (attrs | ...));
        }

        template<Concepts::STDOut Char, LogLevel LoggingLevels>
        static void WriteLog(const LoggingCategory<LoggingLevels, true>& category, const LogLevel& level, const std::basic_string_view<Char>& message)
        {
            auto& _stdcout = Out<char>();
            auto& _stdwout = Out<wchar_t>();

            std::lock_guard lock{ logMutex };

            if(category.writeTime)
            {
                const auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                tm localTime;
                localtime_s(&localTime, &t);

                _stdcout << std::put_time(&localTime, "%H:%M:%S") << ' '; // print timestamp
            }

            _stdcout << category.name << ": ";

            using Background = ConsoleBackgroundColor;
            using Text = ConsoleForegroundColor;
            if(level == LogLevel::Info && category.CanSupportLogLevel(LogLevel::Info))
            {
                SetConsoleColorAttributes(Background::Cyan, Text::White);
                _stdcout << "[INFO]";
                SetConsoleColorAttributes(Text::Cyan, Background::Black);
            }
            else if(level == LogLevel::Warning && category.CanSupportLogLevel(LogLevel::Warning))
            {
                SetConsoleColorAttributes(Background::Yellow, Text::White);
                _stdcout << "[WARNING]";
                SetConsoleColorAttributes(Text::Yellow, Background::Black);
            }
            else if(level == LogLevel::Error && category.CanSupportLogLevel(LogLevel::Error))
            {
                SetConsoleColorAttributes(Background::Red, Text::White);
                _stdcout << "[ERROR]";
                SetConsoleColorAttributes(Text::Red, Background::Black);
            }
            else
                Throw(Format("Logger::WriteLog: invalid logging level or \"", category.name, "\" doesn't support any logging level"), __FILE__, __LINE__);

            _stdcout << ": ";

            if constexpr(std::is_same_v<Char, char>)
                _stdcout << message;
            else
                _stdwout << message;

            _stdcout << '\n';

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
    template<LogLevel LoggingLevels, bool enable, Concepts::STDOut... Args>
    constexpr void Log(const LoggingCategory<LoggingLevels, enable>& category, const LogLevel& level, Args&&... info)
    {
        Logger::Log<LoggingLevels>(category, level, std::forward<Args>(info)...);
    }

    //Added those just because it writes simpler rather than GE_LOG(Core, Info, ...)

    template<typename... Args>
    constexpr void LogInfo(Args&&... info)
    {
        GE_LOG(Core, Info, std::forward<Args>(info)...);
    }
    template<typename ...Args>
    constexpr void LogWarning(Args&&... info)
    {
        GE_LOG(Core, Warning, std::forward<Args>(info)...);
    }
    template<typename ...Args>
    constexpr void LogError(Args&&... info)
    {
        GE_LOG(Core, Error, std::forward<Args>(info)...);
    }

    template<Concepts::STDOut... Args>
    constexpr __forceinline auto Format(Args&&... message)
    {
        return Logger::Format(std::forward<Args>(message)...);
    }
}