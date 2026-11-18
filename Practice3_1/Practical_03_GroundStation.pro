QT += widgets
CONFIG += c++17
TARGET = Practical_03_GroundStation

qtHaveModule(charts) {
    QT += charts
    DEFINES += HAVE_QT_CHARTS
}
qtHaveModule(multimedia) {
    QT += multimedia
    DEFINES += HAVE_QT_MULTIMEDIA
}
msvc: QMAKE_CXXFLAGS += /utf-8

SOURCES += main.cpp mainwindow.cpp uav.cpp sensor.cpp navigation.cpp controller.cpp \
           autopilot.cpp soundengine.cpp fallbackchart.cpp
HEADERS += mainwindow.h uav.h sensor.h navigation.h controller.h autopilot.h \
           soundengine.h fallbackchart.h
FORMS   += mainwindow.ui
