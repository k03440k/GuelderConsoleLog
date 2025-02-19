#include "../include/GuelderConsoleLog.hpp"

#include "../include/GuelderConsoleLogMacroses.hpp"

#include <mutex>
#include <chrono>

namespace GuelderConsoleLog
{
    std::mutex Logger::logMutex;
    HANDLE Logger::console = GetStdHandle(STD_OUTPUT_HANDLE);
    GE_DEFINE_LOG_CATEGORY(Core);

    const Logger::LoggerHelper Logger::loggerHelper = LoggerHelper{};

    Logger::LoggerHelper::LoggerHelper()
    {
        std::setlocale(LC_CTYPE, GE_LOCALE);
#ifdef WIN32
        SetConsoleOutputCP(CP_UTF8);
#endif
    }
}