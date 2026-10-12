cmake_minimum_required(VERSION 3.20)
project(ShareW VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus)

add_executable(shareW
    src/main.cpp
    src/capture/CaptureBackend.cpp
    src/capture/PortalBackend.cpp
    src/capture/RegionSelector.cpp
    src/editor/Canvas.cpp
    src/editor/EditorWindow.cpp
)

target_include_directories(shareW PRIVATE src)
target_link_libraries(shareW PRIVATE Qt6::Widgets Qt6::DBus)

if(MSVC)
    target_compile_options(shareW PRIVATE /W4)
else()
    target_compile_options(shareW PRIVATE -Wall -Wextra)
endif()

install(TARGETS shareW RUNTIME DESTINATION bin)
