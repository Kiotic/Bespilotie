QT += widgets
CONFIG += c++17
TARGET = Control_Work_02_Polymorphism

qtHaveModule(multimedia) {
    QT += multimedia
    DEFINES += HAVE_QT_MULTIMEDIA
}
msvc: QMAKE_CXXFLAGS += /utf-8

SOURCES += main.cpp uav.cpp sensor.cpp navigation.cpp controller.cpp autopilot.cpp \
           tasks.cpp simulation.cpp widgets.cpp soundengine.cpp dashboard.cpp
HEADERS += uav.h sensor.h navigation.h controller.h autopilot.h \
           tasks.h simulation.h widgets.h soundengine.h dashboard.h
