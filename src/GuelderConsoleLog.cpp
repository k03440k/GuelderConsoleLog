#include "../include/GuelderConsoleLog.hpp"

#include <mutex>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <string>
#include <chrono>

namespace GuelderConsoleLog
{
	std::mutex Logger::logMutex;
    HANDLE Logger::console = GetStdHandle(STD_OUTPUT_HANDLE);
	
	void Logger::Throw(const std::string_view& message, const char* const fileName, const uint32_t& line)
    {
        throw(std::runtime_error(Format(message, '\n', "file: ", fileName, ", line: ", line)));
    }
    void Logger::Throw(const std::string_view& message)
    {
        throw(std::runtime_error(message.data()));
    }
    void Logger::Assert(const bool& condition, const std::string& message, const char* const file, const uint32_t& line)
    {
        if(!condition)
        {
            Throw(message, file, line);
            __debugbreak();
        }
    }
}