cmake_minimum_required(VERSION 3.16)
project(Penguin VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets DBus Sql OpenGL OpenGLWidgets Test)
find_package(Qt6 OPTIONAL_COMPONENTS Svg)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/desktop
    ${CMAKE_CURRENT_SOURCE_DIR}/src/library
)

set(CORE_SOURCES
    src/core/TimecodeFormatter.cpp
    src/core/LrcParser.cpp
    src/core/EqualizerDSP.cpp
    src/core/VUMeterDSP.cpp
    src/core/SubtitleLoader.cpp
    src/core/MpvBackend.cpp
    src/core/PlaybackEngine.cpp
)

set(CORE_HEADERS
    include/mpv/client.h
    include/mpv/render.h
    include/mpv/render_gl.h
    src/core/TimecodeFormatter.h
    src/core/LrcParser.h
    src/core/EqualizerDSP.h
    src/core/VUMeterDSP.h
    src/core/SubtitleLoader.h
    src/core/MpvBackend.h
    src/core/PlaybackEngine.h
)

set(UI_SOURCES
    src/ui/BrutalistTheme.cpp
    src/ui/TickScrubberWidget.cpp
    src/ui/VUMeterWidget.cpp
    src/ui/EqualizerRackWidget.cpp
    src/ui/TeleprompterWidget.cpp
    src/ui/PlaylistMatrixWidget.cpp
    src/ui/DiagnosticsHUDWidget.cpp
    src/ui/ViewfinderWidget.cpp
    src/ui/AudioDeckWidget.cpp
    src/ui/MainWindow.cpp
)

set(UI_HEADERS
    src/ui/BrutalistTheme.h
    src/ui/TickScrubberWidget.h
    src/ui/VUMeterWidget.h
    src/ui/EqualizerRackWidget.h
    src/ui/TeleprompterWidget.h
    src/ui/PlaylistMatrixWidget.h
    src/ui/DiagnosticsHUDWidget.h
    src/ui/ViewfinderWidget.h
    src/ui/AudioDeckWidget.h
    src/ui/MainWindow.h
)

set(DESKTOP_SOURCES
    src/desktop/CommandLineParser.cpp
    src/desktop/DBusService.cpp
    src/desktop/MPRIS2Adaptor.cpp
)

set(DESKTOP_HEADERS
    src/desktop/CommandLineParser.h
    src/desktop/DBusService.h
    src/desktop/MPRIS2Adaptor.h
)

set(LIBRARY_SOURCES
    src/library/DatabaseManager.cpp
    src/library/StatePersistence.cpp
    src/library/PlaylistManager.cpp
)

set(LIBRARY_HEADERS
    src/library/DatabaseManager.h
    src/library/StatePersistence.h
    src/library/PlaylistManager.h
)

add_library(penguin_core STATIC
    ${CORE_SOURCES} ${CORE_HEADERS}
    ${UI_SOURCES} ${UI_HEADERS}
    ${DESKTOP_SOURCES} ${DESKTOP_HEADERS}
    ${LIBRARY_SOURCES} ${LIBRARY_HEADERS}
)

target_link_libraries(penguin_core PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Qt6::Sql
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    /usr/lib/x86_64-linux-gnu/libmpv.so.2
    m
)

if (TARGET Qt6::Svg)
    target_link_libraries(penguin_core PUBLIC Qt6::Svg)
endif()

# Main Application Executable
add_executable(penguin
    src/main.cpp
)

target_link_libraries(penguin PRIVATE
    penguin_core
)

# Milestone 1 Core Unit Tests
add_executable(test_m1_core
    tests/test_m1_core.cpp
)

target_link_libraries(test_m1_core PRIVATE
    penguin_core
    Qt6::Test
)

add_test(NAME test_m1_core COMMAND test_m1_core)

# Milestone 2 UI Unit Tests
add_executable(test_m2_ui
    tests/test_m2_ui.cpp
)

target_link_libraries(test_m2_ui PRIVATE
    penguin_core
    Qt6::Test
)

add_test(NAME test_m2_ui COMMAND test_m2_ui)

# Milestone 3 & 4 Desktop and Persistence Unit Tests
add_executable(test_m3_m4
    tests/test_m3_m4.cpp
)

target_link_libraries(test_m3_m4 PRIVATE
    penguin_core
    Qt6::Test
)

add_test(NAME test_m3_m4 COMMAND test_m3_m4)
