cmake_minimum_required(VERSION 3.16)
project(Practical_04_MavlinkGroundStation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} QUIET COMPONENTS Charts)
find_package(Qt${QT_VERSION_MAJOR} QUIET COMPONENTS Multimedia)

add_executable(Practical_04_MavlinkGroundStation
    main.cpp
    mainwindow.h mainwindow.cpp mainwindow.ui
    telemetrymodel.h telemetrymodel.cpp
    mavlinkmanager.h mavlinkmanager.cpp
    mavlinkmessageparser.h mavlinkmessageparser.cpp
    uavsimulator.h uavsimulator.cpp
    soundengine.h soundengine.cpp
    signalbars.h signalbars.cpp
    fallbackchart.h fallbackchart.cpp
)

target_link_libraries(Practical_04_MavlinkGroundStation PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network)

if(Qt${QT_VERSION_MAJOR}Charts_FOUND)
    target_link_libraries(Practical_04_MavlinkGroundStation PRIVATE Qt${QT_VERSION_MAJOR}::Charts)
    target_compile_definitions(Practical_04_MavlinkGroundStation PRIVATE HAVE_QT_CHARTS)
endif()

if(Qt${QT_VERSION_MAJOR}Multimedia_FOUND)
    target_link_libraries(Practical_04_MavlinkGroundStation PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia)
    target_compile_definitions(Practical_04_MavlinkGroundStation PRIVATE HAVE_QT_MULTIMEDIA)
endif()
