QT += core gui widgets dbus sql opengl openglwidgets testlib

CONFIG += c++17
CONFIG += console
CONFIG -= app_bundle

INCLUDEPATH += $$PWD/../include $$PWD/../src $$PWD/../src/core

LIBS += -L/usr/lib/x86_64-linux-gnu -l:libmpv.so.2 -lm

HEADERS += \
    ../include/mpv/client.h \
    ../include/mpv/render.h \
    ../include/mpv/render_gl.h \
    ../src/core/TimecodeFormatter.h \
    ../src/core/LrcParser.h \
    ../src/core/EqualizerDSP.h \
    ../src/core/VUMeterDSP.h \
    ../src/core/SubtitleLoader.h \
    ../src/core/MpvBackend.h \
    ../src/core/PlaybackEngine.h

SOURCES += \
    ../src/core/TimecodeFormatter.cpp \
    ../src/core/LrcParser.cpp \
    ../src/core/EqualizerDSP.cpp \
    ../src/core/VUMeterDSP.cpp \
    ../src/core/SubtitleLoader.cpp \
    ../src/core/MpvBackend.cpp \
    ../src/core/PlaybackEngine.cpp \
    test_m1_core.cpp

TARGET = test_m1_core
