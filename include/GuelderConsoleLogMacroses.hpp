#pragma once

//#define GE_NO_DEBUG

#ifdef _DEBUG
#ifndef GE_NO_DEBUG

#define GE_DEBUG

#endif
#endif

#ifndef GE_LOCALE
#define GE_LOCALE ""
#endif

#define GE_FUNC_NAME __func__

#if defined(_MSC_VER)
#define GE_FULL_FUNC_NAME __FUNCSIG__
#elif defined(__GNUC__) || defined(__clang__)
#define GE_FULL_FUNC_NAME __PRETTY_FUNCTION__
#else
#define GE_FULL_FUNC_NAME GE_FUNC_NAME
#endif

#define GE_MSG_METHOD_LOGGING(...) ::GuelderConsoleLog::Logger::Format(GE_FULL_FUNC_NAME, ": ", __VA_ARGS__)

/**
 * \brief Throws an std::exception.
 * \param ... Message(any type that supports "<<" operator for std::cout), which will be put inside an std::exception.
 */
#define GE_THROW(...) ::GuelderConsoleLog::Logger::Throw(::GuelderConsoleLog::Logger::Format(GE_FULL_FUNC_NAME, ": ", __VA_ARGS__), __FILE__, __LINE__)
/**
 * \brief Throws something.
 * \param exception Some object, which will be thrown.
 */
#define GE_CTHROW(exception) ::GuelderConsoleLog::Logger::Throw(exception)

/**
 * \brief If condition is false then it will throw exception. Prints the path of the file and message(msg).
 * \param condition If false -> throw an error.
 * \param ... Message, which will be put inside std::exception.
 */
#define GE_ASSERT(condition, ...) ::GuelderConsoleLog::Logger::Assert(condition, ::GuelderConsoleLog::Logger::Format(GE_FULL_FUNC_NAME, ": ", __VA_ARGS__), __FILE__, __LINE__)
/**
 * \brief If condition is false then it will throw exception. Prints the path of the file and message(msg).
 * \param condition If false -> throw an error.
 * \param exception Some object, which will be thrown.
 */
#define GE_CASSERT(condition, exception) ::GuelderConsoleLog::Logger::Assert(condition, exception)

#define GE_TO_STRING(arg) #arg
#define GE_CONCATENATE_IMPL(a, b) a##b
#define GE_CONCATENATE(a, b) GE_CONCATENATE_IMPL(a, b)
#define GE_CONCATENATE_STRING(lhs, rhs) GE_TO_STRING(GE_CONCATENATE(lhs, rhs))

#define GE_MAKE_FULL_ERROR_STRING(message) ::GuelderConsoleLog::Logger::Format(GE_FULL_FUNC_NAME, ": ", message, '\n', "file: ", __FILE__, ", line: ", __LINE__)

#define GE_LOG_CATEGORY_TYPE(name) GE_CONCATENATE(name, LoggingCategory)
#define GE_LOG_CATEGORY_VARIABLE(name) GE_CONCATENATE(name, LoggingCategoryVar)

#define GE_LOG_LEVELS_COLORS_VARIABLE(name) GE_CONCATENATE(name, LoggingLevelsColors)


#define GE_DECLARE_LOG_LEVELS_SAME_COLORS_CONSTEXPR(name, info, warning, error) GE_DECLARE_LOG_LEVELS_COLORS_CONSTEXPR(name, info, info, warning, warning, error, error)

#define GE_DECLARE_LOG_CATEGORY_DEFAULT_COLORS_CONSTEXPR(name, loggingLevels, enable, debugOnly, writeTime) GE_DECLARE_LOG_CATEGORY_CONSTEXPR(name, loggingLevels, enable, debugOnly, writeTime, ::GuelderConsoleLog::Core)


#if defined(GE_DEBUG) && !defined(GE_NO_DEBUG)

/**
 * \brief Creates a logging type for a constexpr variable, which is also created here.
 * \param name Name of the logging category.
 * \param loggingLevels Supported logging levels.
 * \param enable Whether the logging category works.
 * \param debugOnly Whether this logging category works, when GE_DEBUG and GE_NO_DEBUG are not defined.
 * \param writeTime Whether put time before the message being output.
 * \param colors Colors, which logging category will be using(If you don't care use GE_DECLARE_LOG_CATEGORY_DEFAULT_COLORS_CONSTEXPR).
 */
#define GE_DECLARE_LOG_CATEGORY_CONSTEXPR(name, loggingLevels, enable, debugOnly, writeTime, colors)\
    struct GE_LOG_CATEGORY_TYPE(name) final : ::GuelderConsoleLog::LoggingCategory<::GuelderConsoleLog::LogLevel::##loggingLevels, enable, writeTime, GE_LOG_LEVELS_COLORS_VARIABLE(colors)>\
    {\
        constexpr GE_LOG_CATEGORY_TYPE(name)() : ::GuelderConsoleLog::LoggingCategory<::GuelderConsoleLog::LogLevel::##loggingLevels, enable, writeTime, GE_LOG_LEVELS_COLORS_VARIABLE(colors)>(GE_TO_STRING(name)) {}\
    };\
        constexpr GE_LOG_CATEGORY_TYPE(name) GE_LOG_CATEGORY_VARIABLE(name)

#endif

#if !defined(GE_DEBUG) && !defined(GE_NO_DEBUG)

/**
 * \brief Creates a logging type for a constexpr variable, which is also created here.
 * \param name Name of the logging category.
 * \param loggingLevels Supported logging levels.
 * \param enable Whether the logging category works.
 * \param debugOnly Whether this logging category works, when GE_DEBUG and GE_NO_DEBUG are not defined.
 * \param writeTime Whether put time before the message being output.
 * \param colors Colors, which logging category will be using(If you don't care use GE_DECLARE_LOG_CATEGORY_DEFAULT_COLORS_CONSTEXPR).
 */
#define GE_DECLARE_LOG_CATEGORY_CONSTEXPR(name, loggingLevels, enable, debugOnly, writeTime, colors)\
    struct GE_LOG_CATEGORY_TYPE(name) final : ::GuelderConsoleLog::LoggingCategory<::GuelderConsoleLog::LogLevel::##loggingLevels, enable && !debugOnly, writeTime, GE_LOG_LEVELS_COLORS_VARIABLE(colors)>\
    {\
        constexpr GE_LOG_CATEGORY_TYPE(name)() : ::GuelderConsoleLog::LoggingCategory<::GuelderConsoleLog::LogLevel::##loggingLevels, enable && !debugOnly, writeTime, GE_LOG_LEVELS_COLORS_VARIABLE(colors)>(GE_TO_STRING(name)) {}\
    };\
        constexpr GE_LOG_CATEGORY_TYPE(name) GE_LOG_CATEGORY_VARIABLE(name)

#endif

#ifdef GE_NO_DEBUG

#define GE_DECLARE_LOG_LEVELS_COLORS_CONSTEXPR(...)

#define GE_DECLARE_LOG_CATEGORY_CONSTEXPR(...)

#define GE_LOG(...)

#else

#define GE_DECLARE_LOG_LEVELS_COLORS_CONSTEXPR(name, infoCategory, infoMessage, warningCategory, warningMessage, errorCategory, errorMessage) Colors::CategoryColors<Colors::CategoryColor<infoCategory, infoMessage>{}, Colors::CategoryColor<warningCategory, warningMessage>{}, Colors::CategoryColor<errorCategory, errorMessage>{}> constexpr GE_LOG_LEVELS_COLORS_VARIABLE(name)

#define GE_LOG(categoryName, level, ...) ::GuelderConsoleLog::Log<GE_LOG_CATEGORY_TYPE(categoryName)::supportedLoggingLevels, GE_LOG_CATEGORY_TYPE(categoryName)::enable, GE_LOG_CATEGORY_TYPE(categoryName)::writeTime, GE_LOG_CATEGORY_TYPE(categoryName)::levelsColors>(GE_LOG_CATEGORY_VARIABLE(categoryName), ::GuelderConsoleLog::LogLevel::##level, __VA_ARGS__)

#endif