# GuelderConsoleLog
A simple logging library that uses standart output to display the message to the console. It is recommended to use windows as color features are available only with windows.

## Features

- Possibility of adding custom logging categories.
- Possibility of adding custom colors to logging categories.

Use CMake to build this library with your main project. You need to download this code and do the following inside your CMakeLists.txt:

```
add_subdirectory("External/GuelderConsoleLog" "${CMAKE_CURRENT_BINARY_DIR}/GuelderConsoleLog")
target_link_libraries(${PROJECT_NAME} PUBLIC GuelderConsoleLog)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/External/GuelderConsoleLog/include")
```