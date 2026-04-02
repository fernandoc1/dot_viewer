cmake_minimum_required(VERSION 3.16)
project(DotViewer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(SOURCES
    src/main.cpp
    src/dotparser.cpp
    src/graphmodel.cpp
    src/mainwindow.cpp
)

set(HEADERS
    include/dotparser.h
    include/graphmodel.h
    include/mainwindow.h
)

add_executable(dot_viewer ${SOURCES} ${HEADERS})

target_include_directories(dot_viewer PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(dot_viewer PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
