cmake_minimum_required(VERSION 3.16)
project(Practical_03_GroundStation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} QUIET COMPONENTS Charts)
find_package(Qt${QT_VERSION_MAJOR} QUIET COMPONENTS Multimedia)

add_executable(Practical_03_GroundStation
    main.cpp
    mainwindow.h mainwindow.cpp mainwindow.ui
    uav.h uav.cpp
    sensor.h sensor.cpp
    navigation.h navigation.cpp
    controller.h controller.cpp
    autopilot.h autopilot.cpp
    soundengine.h soundengine.cpp
    fallbackchart.h fallbackchart.cpp
)

target_link_libraries(Practical_03_GroundStation PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(Qt${QT_VERSION_MAJOR}Charts_FOUND)
    target_link_libraries(Practical_03_GroundStation PRIVATE Qt${QT_VERSION_MAJOR}::Charts)
    target_compile_definitions(Practical_03_GroundStation PRIVATE HAVE_QT_CHARTS)
endif()
if(Qt${QT_VERSION_MAJOR}Multimedia_FOUND)
    target_link_libraries(Practical_03_GroundStation PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia)
    target_compile_definitions(Practical_03_GroundStation PRIVATE HAVE_QT_MULTIMEDIA)
endif()
