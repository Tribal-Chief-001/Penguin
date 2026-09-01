QT += core gui widgets testlib

CONFIG += c++17
CONFIG += console
CONFIG -= app_bundle

INCLUDEPATH += $$PWD/../include $$PWD/../src $$PWD/../src/core

HEADERS += \
    ../src/core/TimecodeFormatter.h \
    ../src/core/LrcParser.h

SOURCES += \
    ../src/core/TimecodeFormatter.cpp \
    ../src/core/LrcParser.cpp \
    test_challenger_m1.cpp

TARGET = test_challenger_m1
