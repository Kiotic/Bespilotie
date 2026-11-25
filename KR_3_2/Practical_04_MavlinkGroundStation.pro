QT += widgets network
CONFIG += c++17
TARGET = Practical_04_MavlinkGroundStation

qtHaveModule(charts) {
    QT += charts
    DEFINES += HAVE_QT_CHARTS
}
qtHaveModule(multimedia) {
    QT += multimedia
    DEFINES += HAVE_QT_MULTIMEDIA
}
msvc: QMAKE_CXXFLAGS += /utf-8

SOURCES += main.cpp \
           mainwindow.cpp \
           telemetrymodel.cpp \
           mavlinkmanager.cpp \
           mavlinkmessageparser.cpp \
           uavsimulator.cpp \
           soundengine.cpp \
           signalbars.cpp \
           fallbackchart.cpp

HEADERS += mainwindow.h \
           telemetrymodel.h \
           mavlinkmanager.h \
           mavlinkmessageparser.h \
           uavsimulator.h \
           soundengine.h \
           signalbars.h \
           fallbackchart.h

FORMS += mainwindow.ui
