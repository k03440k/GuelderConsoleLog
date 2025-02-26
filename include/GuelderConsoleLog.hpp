#pragma once

#include "GuelderConsoleLogMacroses.hpp"

#ifdef WIN32
#include <Windows.h>
#endif

#include <iostream>
#include <mutex>
#include <string_view>
#include <string>
#include <sstream>
#include <chrono>
#include <type_traits>
#include <functional>

//enums of color attributes
namespace GuelderConsoleLog
{
    enum class ConsoleForegroundColor : uint16_t
    {
        Black = 0
#ifdef WIN32
        ,
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
#endif
    };

    enum class ConsoleBackgroundColor : uint16_t
    {
        Black = 0
#ifdef WIN32
        ,
        Blue = BACKGROUND_BLUE,
        Green = BACKGROUND_GREEN,
        Cyan = BACKGROUND_GREEN | BACKGROUND_BLUE,
        Red = BACKGROUND_RED,
        Magenta = BACKGROUND_RED | BACKGROUND_BLUE,
        Yellow = BACKGROUND_RED | BACKGROUND_GREEN,
        White = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE
#endif
    };
}

//strings converting
namespace GuelderConsoleLog
{
    /**
     * \brief WARNING: This func only works properly with windows, because I don't give a fuck about Linux or MacOS.
     */
    inline std::wstring StringToWString(const std::string_view& str)
    {
#ifdef WIN32
        const int neededSize = MultiByteToWideChar(CP_UTF8, 0, str.data(), - 1, nullptr, 0);
        std::wstring wString(neededSize, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, str.data(), -1, wString.data(), neededSize);
#else
        std::wstring wString{};
#endif

        return wString;
    }
    /**
     * \brief WARNING: This func only works properly with windows, because I don't give a fuck about Linux or MacOS.
     */
    inline std::string WStringToString(const std::wstring_view& wStr)
    {
#ifdef WIN32
        const int neededSize = WideCharToMultiByte(CP_UTF8, 0, wStr.data(), wStr.size(), nullptr, 0, nullptr, nullptr);
        std::string string(neededSize, L'\0');
        WideCharToMultiByte(CP_UTF8, 0, wStr.data(), wStr.size(), string.data(), neededSize, nullptr, nullptr);
#else
        std::string string;
#endif

        return string;
    }
}

//concepts
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
        concept IsAtLeastOneSameRaw = (SameRawType<RawType<String>, Strings> || ...);

        template<typename Char, typename T>
        concept HasOutputOperator = (requires(std::basic_ostream<Char>&os, const T & t) { os << t; });

        template<typename T>
        concept STDCout = HasOutputOperator<char, T>;

        template<typename T>
        concept STDWout = HasOutputOperator<wchar_t, T> && !STDCout<T>;

        template<typename T>
        concept STDOut = STDCout<T> || STDWout<T>;

        template<typename... Attributes>
        concept ConsoleColorAttributes = (std::is_same_v<RawType<Attributes>, ConsoleForegroundColor> || ...) || (std::is_same_v<RawType<Attributes>, ConsoleBackgroundColor> || ...) || (std::is_same_v<RawType<Attributes>, uint16_t> || ...);

        template<typename T>
        concept IsException = (std::is_same_v<T, std::exception> || std::is_base_of_v<std::exception, T>) && (std::is_constructible_v<T, const std::string&> || std::is_constructible_v<T, const char*>);
    }
}

//colors stuff
namespace GuelderConsoleLog
{
    template<Concepts::ConsoleColorAttributes Rhs, Concepts::ConsoleColorAttributes Lhs>
    constexpr uint16_t operator|(Rhs&& lhs, Lhs&& rhs)
    {
        return static_cast<std::underlying_type_t<ConsoleForegroundColor>>(lhs) | static_cast<std::underlying_type_t<ConsoleForegroundColor>>(rhs);
    }

    /**
     * \brief Struct, which is used to set colors of a log message
     * \tparam _categoryColor The color, which is used with logging category name.
     * \tparam _messageColor The color, which is used with the actual message.
     */
    template<uint16_t _categoryColor, uint16_t _messageColor>
    struct LoggingLevelsCategoryColor final
    {
        static constexpr uint16_t categoryColor = _categoryColor;
        static constexpr uint16_t messageColor = _messageColor;
    };

    /**
     * \brief Struct, which represents a set of LoggingLevelsCategoryColor for Info, Warning and Error logging categories.
     */
    template<LoggingLevelsCategoryColor _info, LoggingLevelsCategoryColor _warning, LoggingLevelsCategoryColor _error>
    struct LoggingLevelsCategoryColors final
    {
        static constexpr LoggingLevelsCategoryColor info = _info;
        static constexpr LoggingLevelsCategoryColor warning = _warning;
        static constexpr LoggingLevelsCategoryColor error = _error;
    };

    //aliases
    namespace Colors
    {
        using Background = ConsoleBackgroundColor;
        using Text = ConsoleForegroundColor;

        template<uint16_t _categoryColor, uint16_t _messageColor>
        using CategoryColor = LoggingLevelsCategoryColor<_categoryColor, _messageColor>;

        template<CategoryColor _info, CategoryColor _warning, CategoryColor _error>
        using CategoryColors = LoggingLevelsCategoryColors<_info, _warning, _error>;
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

    template<LogLevel loggingLevels, bool _enable, bool _writeTime, Colors::CategoryColors _levelsColors>
    struct LoggingCategory {};

    template<LogLevel loggingLevels, bool _writeTime, Colors::CategoryColors _levelsColors>
    struct LoggingCategory<loggingLevels, false, _writeTime, _levelsColors>
    {
        constexpr LoggingCategory(const std::string_view& name) {}

        static constexpr LogLevel supportedLoggingLevels = loggingLevels;
        static constexpr bool enable = false;
        static constexpr bool writeTime = _writeTime;

        static constexpr Colors::CategoryColors levelsColors = _levelsColors;
    };

    template<LogLevel loggingLevels, bool _writeTime, Colors::CategoryColors _levelsColors>
    struct LoggingCategory<loggingLevels, true, _writeTime, _levelsColors>
    {
        constexpr LoggingCategory(const std::string_view& name)
            : name(name) {}
        virtual ~LoggingCategory() = default;

        [[nodiscard]]
        static constexpr bool CanSupportLogLevel(const LogLevel& level)
        {
            if(supportedLoggingLevels == LogLevel::All)
                return true;
            return static_cast<std::underlying_type_t<LogLevel>>(level) &
                static_cast<std::underlying_type_t<LogLevel>>(supportedLoggingLevels);
        }

        static constexpr LogLevel supportedLoggingLevels = loggingLevels;
        std::string_view name;
        static constexpr bool enable = true;
        static constexpr bool writeTime = _writeTime;

        static constexpr Colors::CategoryColors levelsColors = _levelsColors;
    };

    class Logger final
    {
    public:
        Logger() = delete;
        ~Logger() = delete;

        template<LogLevel loggingLevels, Colors::CategoryColors _levelsColors, bool writeTime, Concepts::STDOut... Args>
        constexpr static void Log(const LoggingCategory<loggingLevels, true, writeTime, _levelsColors>& category, const LogLevel& level, Args&&... args)
        {
            if constexpr(Concepts::IsThereAtLeastOneWideChar<Args...>)
                WriteLog<wchar_t, loggingLevels, writeTime, _levelsColors>(category, level, Format(std::forward<Args>(args)...));
            else
                WriteLog<char, loggingLevels, writeTime, _levelsColors>(category, level, Format(std::forward<Args>(args)...));
        }
        /// <summary>
        /// disabled if enable == false
        /// </summary>
        template<LogLevel loggingLevels, Colors::CategoryColors _levelsColors, bool writeTime, Concepts::STDOut... Args>
        constexpr static void Log(const LoggingCategory<loggingLevels, false, writeTime, _levelsColors>& category, const LogLevel& level, Args&&... args) {}

        template<Concepts::IsException Exception = std::exception>
        [[noreturn]]
        static void Throw(const std::string_view& message, const char* const fileName, const uint32_t& line)
        {
            throw Exception(Format(message, '\n', "file: ", fileName, ", line: ", line).c_str());
        }

        template<Concepts::IsException Exception = std::exception>
        [[noreturn]]
        static void Throw(const std::string_view& message)
        {
            throw Exception(message.data());
        }

        template<typename Exception = std::exception>
        [[noreturn]]
        static void Throw(Exception&& exception)
        {
            throw exception;
        }

        /**
         * \brief if input bool is false, then it will bring throw of runtime_error
         */
        template<Concepts::IsException Exception = std::exception>
        static void Assert(const bool& condition, const std::string_view& message = "", const char* file = __FILE__, const uint32_t& line = __LINE__)
        {
            if(!condition)
                Throw<Exception>(message, file, line);
        }
        /**
         * \brief if input bool is false, then it will bring throw of runtime_error
         */
        template<typename Exception = std::exception>
        static void Assert(const bool& condition, Exception&& exception)
        {
            if(!condition)
                Throw(std::forward<Exception>(exception));
        }

        /**
         * \return std::string or std::wstring, which were was made from all the input params.
         */
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
        struct LoggerHelper final
        {
            LoggerHelper();
        };

    private:
        static std::mutex logMutex;
#ifdef WIN32
        static HANDLE console;
#endif

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
        
#ifdef WIN32
        template<Concepts::ConsoleColorAttributes... Attributes>
        constexpr static void SetConsoleColorAttributes(Attributes&&... attrs)
        {
            SetConsoleTextAttribute(console, static_cast<uint16_t>((static_cast<uint16_t>(attrs) | ...)));
        }
#endif

        template<Concepts::STDOut Char, LogLevel loggingLevels, bool writeTime, Colors::CategoryColors levelsColors>
        constexpr static void WriteLog(const LoggingCategory<loggingLevels, true, writeTime, levelsColors>& category, const LogLevel& level, const std::basic_string_view<Char>& message)
        {
            auto& _stdcout = Out<char>();
            auto& _stdwout = Out<wchar_t>();

            std::lock_guard lock{ logMutex };

            if constexpr(category.writeTime)
            {
                const auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                tm localTime;
                localtime_s(&localTime, &t);

                _stdcout << std::put_time(&localTime, "%H:%M:%S") << ' '; // print timestamp
            }

            _stdcout << category.name << ": ";

            using Background = ConsoleBackgroundColor;
            using Text = ConsoleForegroundColor;
#ifdef WIN32
            if(level == LogLevel::Info && category.CanSupportLogLevel(LogLevel::Info))
            {
                SetConsoleColorAttributes(category.levelsColors.info.categoryColor);
                _stdcout << "[INFO]";
                SetConsoleColorAttributes(category.levelsColors.info.messageColor);
            }
            else if(level == LogLevel::Warning && category.CanSupportLogLevel(LogLevel::Warning))
            {
                SetConsoleColorAttributes(category.levelsColors.warning.categoryColor);
                _stdcout << "[WARNING]";
                SetConsoleColorAttributes(category.levelsColors.warning.messageColor);
            }
            else if(level == LogLevel::Error && category.CanSupportLogLevel(LogLevel::Error))
            {
                SetConsoleColorAttributes(category.levelsColors.error.categoryColor);
                _stdcout << "[ERROR]";
                SetConsoleColorAttributes(category.levelsColors.error.messageColor);
            }
#else
            if(level == LogLevel::Info && category.CanSupportLogLevel(LogLevel::Info))
                _stdcout << "[INFO]";
            else if(level == LogLevel::Warning && category.CanSupportLogLevel(LogLevel::Warning))
                _stdcout << "[WARNING]";
            else if(level == LogLevel::Error && category.CanSupportLogLevel(LogLevel::Error))
                _stdcout << "[ERROR]";
#endif
            else
                Throw(Format("Logger::WriteLog: invalid logging level or \"", category.name, "\" doesn't support any logging level"), __FILE__, __LINE__);

            _stdcout << ": ";

            if constexpr(std::is_same_v<Char, char>)
                _stdcout << message;
            else
                _stdwout << message;

            _stdcout << '\n';

#ifdef WIN32
            SetConsoleColorAttributes(Text::White, Background::Black);
#endif
        }
    };
}

//the main logging category type
namespace GuelderConsoleLog
{
#ifdef WIN32
    GE_DECLARE_LOG_LEVELS_COLORS_CONSTEXPR(Core,
        Colors::Background::Cyan | Colors::Text::White, Colors::Background::Black | Colors::Text::Cyan,
        Colors::Background::Yellow | Colors::Text::White, Colors::Background::Black | Colors::Text::Yellow,
        Colors::Background::Red | Colors::Text::White, Colors::Background::Black | Colors::Text::Red);
#else
    GE_DECLARE_LOG_LEVELS_COLORS_CONSTEXPR(Core,
        0, 0,
        0, 0,
        0, 0);
#endif
    
    GE_DECLARE_LOG_CATEGORY_DEFAULT_COLORS_CONSTEXPR(Core, All, true, true, true);
}

//helper functions
namespace GuelderConsoleLog
{
    /// <summary>
    /// Prints into std::cout as custom type.
    /// Another form of Logger::Log(color, categoryName, ...)
    /// </summary>
    template<LogLevel loggingLevels, bool enable, bool writeTime, Colors::CategoryColors _levelsColors, Concepts::STDOut... Args>
    constexpr void Log(const LoggingCategory<loggingLevels, enable, writeTime, _levelsColors>& category, const LogLevel& level, Args&&... info)
    {
        Logger::Log<loggingLevels, _levelsColors, writeTime>(category, level, std::forward<Args>(info)...);
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
}