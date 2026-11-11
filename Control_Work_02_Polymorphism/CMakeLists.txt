cmake_minimum_required(VERSION 3.16)
project(Control_Work_02_Polymorphism LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} QUIET COMPONENTS Multimedia)   # звук (необязательно)

add_executable(Control_Work_02_Polymorphism
    main.cpp
    uav.h uav.cpp
    sensor.h sensor.cpp
    navigation.h navigation.cpp
    controller.h controller.cpp
    autopilot.h autopilot.cpp
    tasks.h tasks.cpp
    simulation.h simulation.cpp
    widgets.h widgets.cpp
    soundengine.h soundengine.cpp
    dashboard.h dashboard.cpp
)

target_link_libraries(Control_Work_02_Polymorphism PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(Qt${QT_VERSION_MAJOR}Multimedia_FOUND)
    target_link_libraries(Control_Work_02_Polymorphism PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia)
    target_compile_definitions(Control_Work_02_Polymorphism PRIVATE HAVE_QT_MULTIMEDIA)
    message(STATUS "Qt Multimedia found: synthesized sound enabled")
else()
    message(STATUS "Qt Multimedia NOT found: fallback to system beep")
endif()
