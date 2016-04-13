TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    AcampProtocol.cpp \
    AcampTransmission.cpp

HEADERS += \
    Common.h \
    AcampProtocol.h \
    AcampTransmission.h

