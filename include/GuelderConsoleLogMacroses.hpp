#pragma once

//#define GE_NO_DEBUG

#ifdef _DEBUG
#ifndef GE_NO_DEBUG

#define GE_DEBUG

#endif
#endif

#define GE_FUNC_NAME __func__

#if defined(_MSC_VER)
#define GE_FULL_FUNC_NAME __FUNCSIG__
#elif defined(__GNUC__) || defined(__clang__)
#define GE_FULL_FUNC_NAME __PRETTY_FUNCTION__
#else
#define GE_FULL_FUNC_NAME FUNC_NAME
#endif

#define GE_MSG_METHOD_LOGGING(...) ::GuelderConsoleLog::Logger::Format(GE_FULL_FUNC_NAME, ": ", __VA_ARGS__)

/// <summary>
/// guelder engine error log
/// </summary>
/// <param name = "...">
/// all debug info(it can be chars, ints, floats, and other types which support '<<' operator)
/// </param>
#define GE_THROW(...) ::GuelderConsoleLog::Logger::Throw(::GuelderConsoleLog::Logger::Format(GE_FULL_FUNC_NAME, ": ", __VA_ARGS__), __FILE__, __LINE__)

/// <summary>
/// If condition(param) is false then it will throw exception. Prints the path of the file and message(msg).
/// </summary>
/// <param name = "...">
/// message, any type that supports "<<" operator
/// </param>
#define GE_ASSERT(condition, ...) ::GuelderConsoleLog::Logger::Assert(condition, ::GuelderConsoleLog::Logger::Format(GE_FULL_FUNC_NAME, ": ", __VA_ARGS__), __FILE__, __LINE__)

#define GE_TO_STRING(arg) #arg
#define GE_CONCATENATE(lhs, rhs) GE_TO_STRING(lhs ## rhs)

#define GE_LOG_CATEGORY_VARIABLE(name) name##LoggingCategoryVar
#define GE_LOG_CATEGORY(name) name##LoggingCategory

#if defined(GE_DEBUG) && !defined(GE_NO_DEBUG)

/// <summary>
/// Creates an extern variable
/// </summary>
/// <param name = "name"> Name of the logging category </param>
/// <param name = "loggingLevels"> Supported logging levels </param>
/// <param name = "enable"> Whether the logging category works </param>
/// <param name = "debugOnly"> Whether this logging category works, when GE_DEBUG and GE_NO_DEBUG are not defined </param>
/// <param name = "writeTime"> Whether put time before the message being output </param>
#define GE_DECLARE_LOG_CATEGORY_EXTERN(name, loggingLevels, enable, debugOnly, writeTime)\
    struct GE_LOG_CATEGORY(name) final : ::GuelderConsoleLog::LoggingCategory<::GuelderConsoleLog::LogLevel::##loggingLevels, enable>\
    {\
        GE_LOG_CATEGORY(name)() : ::GuelderConsoleLog::LoggingCategory<::GuelderConsoleLog::LogLevel::##loggingLevels, enable>(GE_TO_STRING(name), writeTime) {}\
    };\
        extern GE_LOG_CATEGORY(name) GE_LOG_CATEGORY_VARIABLE(name)

#define GE_DEFINE_LOG_CATEGORY(name) GE_LOG_CATEGORY(name) GE_LOG_CATEGORY_VARIABLE(name)

#define GE_LOG(categoryName, level, ...) ::GuelderConsoleLog::Log<GE_LOG_CATEGORY(categoryName)::supportedLoggingLevels, GE_LOG_CATEGORY(categoryName)::enable>(GE_LOG_CATEGORY_VARIABLE(categoryName), ::GuelderConsoleLog::LogLevel::##level, __VA_ARGS__)

#endif

#if !defined(GE_DEBUG) && !defined(GE_NO_DEBUG)

/// <summary>
/// Creates an extern variable
/// </summary>
/// <param name = "name"> Name of the logging category </param>
/// <param name = "loggingLevels"> Supported logging levels </param>
/// <param name = "enable"> Whether the logging category works </param>
/// <param name = "debugOnly"> Whether this logging category works, when GE_DEBUG and GE_NO_DEBUG are not defined </param>
/// <param name = "writeTime"> Whether put time before the message being output </param>
#define GE_DECLARE_LOG_CATEGORY_EXTERN(name, loggingLevels, enable, debugOnly, writeTime)\
    struct GE_LOG_CATEGORY(name) final : ::GuelderConsoleLog::LoggingCategory<::GuelderConsoleLog::LogLevel::##loggingLevels, enable && !debugOnly>\
    {\
        GE_LOG_CATEGORY(name)() : ::GuelderConsoleLog::LoggingCategory<::GuelderConsoleLog::LogLevel::##loggingLevels, enable && !debugOnly>(GE_TO_STRING(name), writeTime) {}\
    };\
        extern GE_LOG_CATEGORY(name) GE_LOG_CATEGORY_VARIABLE(name)

#define GE_DEFINE_LOG_CATEGORY(name) GE_LOG_CATEGORY(name) GE_LOG_CATEGORY_VARIABLE(name)

#define GE_LOG(categoryName, level, ...) ::GuelderConsoleLog::Log<GE_LOG_CATEGORY(categoryName)::supportedLoggingLevels, GE_LOG_CATEGORY(categoryName)::enable>(GE_LOG_CATEGORY_VARIABLE(categoryName), ::GuelderConsoleLog::LogLevel::##level, __VA_ARGS__)

#endif

#ifdef GE_NO_DEBUG

#define GE_DECLARE_LOG_CATEGORY_EXTERN(...)

#define GE_DEFINE_LOG_CATEGORY(...)
#define GE_LOG(...)

#endif