QT += core gui widgets dbus sql opengl openglwidgets

CONFIG += c++17
CONFIG += link_pkgconfig

INCLUDEPATH += $$PWD/include $$PWD/src $$PWD/src/core $$PWD/src/ui $$PWD/src/desktop $$PWD/src/library

# Link libmpv2
LIBS += -L/usr/lib/x86_64-linux-gnu -l:libmpv.so.2 -lm

HEADERS += \
    include/mpv/client.h \
    include/mpv/render.h \
    include/mpv/render_gl.h \
    src/core/TimecodeFormatter.h \
    src/core/LrcParser.h \
    src/core/EqualizerDSP.h \
    src/core/VUMeterDSP.h \
    src/core/SubtitleLoader.h \
    src/core/MpvBackend.h \
    src/core/PlaybackEngine.h \
    src/ui/BrutalistTheme.h \
    src/ui/TickScrubberWidget.h \
    src/ui/VUMeterWidget.h \
    src/ui/EqualizerRackWidget.h \
    src/ui/TeleprompterWidget.h \
    src/ui/PlaylistMatrixWidget.h \
    src/ui/DiagnosticsHUDWidget.h \
    src/ui/ViewfinderWidget.h \
    src/ui/AudioDeckWidget.h \
    src/ui/MainWindow.h \
    src/desktop/CommandLineParser.h \
    src/desktop/DBusService.h \
    src/desktop/MPRIS2Adaptor.h \
    src/library/DatabaseManager.h \
    src/library/StatePersistence.h \
    src/library/PlaylistManager.h

SOURCES += \
    src/main.cpp \
    src/core/TimecodeFormatter.cpp \
    src/core/LrcParser.cpp \
    src/core/EqualizerDSP.cpp \
    src/core/VUMeterDSP.cpp \
    src/core/SubtitleLoader.cpp \
    src/core/MpvBackend.cpp \
    src/core/PlaybackEngine.cpp \
    src/ui/BrutalistTheme.cpp \
    src/ui/TickScrubberWidget.cpp \
    src/ui/VUMeterWidget.cpp \
    src/ui/EqualizerRackWidget.cpp \
    src/ui/TeleprompterWidget.cpp \
    src/ui/PlaylistMatrixWidget.cpp \
    src/ui/DiagnosticsHUDWidget.cpp \
    src/ui/ViewfinderWidget.cpp \
    src/ui/AudioDeckWidget.cpp \
    src/ui/MainWindow.cpp \
    src/desktop/CommandLineParser.cpp \
    src/desktop/DBusService.cpp \
    src/desktop/MPRIS2Adaptor.cpp \
    src/library/DatabaseManager.cpp \
    src/library/StatePersistence.cpp \
    src/library/PlaylistManager.cpp

TARGET = penguin
TEMPLATE = app
