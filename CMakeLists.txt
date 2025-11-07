cmake_minimum_required(VERSION 4.0)
project(curves3D)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt6 COMPONENTS
        Core
        Gui
        OpenGL
        OpenGLWidgets
        Widgets
        REQUIRED)

add_executable(curves3D main.cpp
        PointModel.cpp
        PointModel.h
        CurveCalculator.cpp
        CurveCalculator.h
        DrawingArea.cpp
        DrawingArea.h
        MainWindow.cpp
        MainWindow.h)
target_link_libraries(curves3D
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::OpenGL
        Qt::OpenGLWidgets
)

