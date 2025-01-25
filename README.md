# GuelderConsoleLog
A simple logging library, which currently supports only std::cout. It is possible to create custom logging categories like in UnrealEngine(I just stole the idea ._.).

To link in your CMakeLists.txt use:

`
add_subdirectory("External/GuelderConsoleLog" "${CMAKE_CURRENT_BINARY_DIR}/GuelderConsoleLog")
target_link_libraries(${PROJECT_NAME} PUBLIC GuelderConsoleLog)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/External/GuelderConsoleLog/include")
`