#include "../include/GuelderConsoleLog.hpp"

#include <mutex>

namespace GuelderConsoleLog
{
    std::mutex Logger::logMutex;
#ifdef WIN32
    HANDLE Logger::console = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
    const Logger::LoggerHelper Logger::loggerHelper = LoggerHelper{};

    Logger::LoggerHelper::LoggerHelper()
    {
        std::setlocale(LC_CTYPE, GE_LOCALE);
#ifdef WIN32
        SetConsoleOutputCP(CP_UTF8);
#endif
    }
}