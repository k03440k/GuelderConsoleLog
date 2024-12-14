cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project(GuelderConsoleLog)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_library(GuelderConsoleLog STATIC
	"include/GuelderConsoleLog.hpp"
	"src/GuelderConsoleLog.cpp"
)