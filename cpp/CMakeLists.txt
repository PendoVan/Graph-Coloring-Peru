cmake_minimum_required(VERSION 3.16)

project(GraphColoringPeru VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "C:/Users/acf12/Qt/6.5.3/msvc2019_64")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Charts Test)

# Add all source files here as we build them
set(CORE_SOURCES
    src/core/Graph.cpp
    src/core/CoreUtils.cpp
    src/core/Algorithms.cpp
    src/core/Generators.cpp
    src/core/PeruMap.cpp
    src/benchmark/Benchmark.cpp
)

set(GUI_SOURCES
    src/gui/MainWindow.cpp
    src/gui/MapTab.cpp
    src/gui/DemoTab.cpp
    src/gui/BenchTab.cpp
    src/gui/DocsTab.cpp
)

add_executable(GraphColoringPeru
    src/main.cpp
    ${CORE_SOURCES}
    ${GUI_SOURCES}
)

target_link_libraries(GraphColoringPeru PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Charts Qt6::Test)
