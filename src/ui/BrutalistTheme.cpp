#include "BrutalistTheme.h"
#include <QFontDatabase>
#include <QPalette>
#include <QStyleFactory>

namespace Penguin {
namespace UI {

const QColor BrutalistTheme::BG_DEEP_OBSIDIAN       = QColor("#070709");
const QColor BrutalistTheme::SURFACE_PANEL_BASE     = QColor("#0B0B0E");
const QColor BrutalistTheme::SURFACE_RAISED         = QColor("#141418");
const QColor BrutalistTheme::GRID_STRUCTURAL_BORDER = QColor("#1E1E24");
const QColor BrutalistTheme::GRID_LINE_ACTIVE       = QColor("#333342");
const QColor BrutalistTheme::TEXT_HIGH_CONTRAST     = QColor("#FFFFFF");
const QColor BrutalistTheme::TEXT_SECONDARY_DIM     = QColor("#777788");
const QColor BrutalistTheme::TEXT_MUTED             = QColor("#444455");
const QColor BrutalistTheme::ACCENT_SAFETY_ORANGE   = QColor("#FF4400");
const QColor BrutalistTheme::ACCENT_SIGNAL_LIME     = QColor("#CCFF00");
const QColor BrutalistTheme::ACCENT_TELEMETRY_CYAN  = QColor("#00E5FF");
const QColor BrutalistTheme::ACCENT_CLIP_RED        = QColor("#FF2200");

// Smoked Glass & Laser Glows
const QColor BrutalistTheme::GLASS_SMOKED_OBSIDIAN   = QColor(7, 7, 9, 215);
const QColor BrutalistTheme::GLASS_PANEL_TRANSLUCENT = QColor(14, 14, 20, 220);
const QColor BrutalistTheme::GLASS_BORDER_SUBTLE     = QColor(255, 255, 255, 24);
const QColor BrutalistTheme::GLASS_BORDER_GLOW       = QColor(0, 229, 255, 75);
const QColor BrutalistTheme::GLOW_LASER_ORANGE       = QColor(255, 68, 0, 80);
const QColor BrutalistTheme::GLOW_LASER_LIME         = QColor(204, 255, 0, 80);
const QColor BrutalistTheme::GLOW_LASER_CYAN         = QColor(0, 229, 255, 90);

QFont BrutalistTheme::monospaceFont(int pointSize, QFont::Weight weight)
{
    QStringList families = {"JetBrains Mono", "DejaVu Sans Mono", "Liberation Mono", "Monospace", "Courier New"};
    QFont font;
    font.setFamilies(families);
    font.setPointSize(pointSize);
    font.setWeight(weight);
    font.setStyleHint(QFont::Monospace);
    font.setStyleStrategy(QFont::PreferAntialias);
    return font;
}

QFont BrutalistTheme::sansFont(int pointSize, QFont::Weight weight)
{
    QStringList families = {"Inter", "Liberation Sans", "DejaVu Sans", "Helvetica Neue", "Segoe UI", "sans-serif"};
    QFont font;
    font.setFamilies(families);
    font.setPointSize(pointSize);
    font.setWeight(weight);
    font.setStyleHint(QFont::SansSerif);
    font.setStyleStrategy(QFont::PreferAntialias);
    return font;
}

QString BrutalistTheme::globalStyleSheet()
{
    return QString(R"(
        /* Global Reset & Base */
        * {
            border-radius: 0px;
            outline: none;
            font-family: "Inter", "Liberation Sans", "DejaVu Sans", sans-serif;
        }

        QWidget {
            background-color: #070709;
            color: #FFFFFF;
            selection-background-color: #FF4400;
            selection-color: #FFFFFF;
        }

        QMainWindow {
            background-color: #070709;
        }

        /* Buttons */
        QPushButton {
            background-color: #0B0B0E;
            color: #FFFFFF;
            border: 1px solid #1E1E24;
            padding: 5px 12px;
            font-size: 11px;
            font-weight: bold;
            letter-spacing: 0.5px;
        }

        QPushButton:hover {
            background-color: #141418;
            border: 1px solid #333342;
            color: #FFFFFF;
        }

        QPushButton:pressed {
            background-color: #1E1E24;
            border: 1px solid #FF4400;
            color: #FF4400;
        }

        QPushButton:checked {
            background-color: #1E1E24;
            border: 1px solid #FF4400;
            color: #FF4400;
        }

        QPushButton:disabled {
            background-color: #070709;
            color: #444455;
            border: 1px solid #141418;
        }

        /* Combo Box */
        QComboBox {
            background-color: #0B0B0E;
            color: #FFFFFF;
            border: 1px solid #1E1E24;
            padding: 4px 8px;
            font-size: 11px;
            min-height: 22px;
        }

        QComboBox:hover {
            border: 1px solid #333342;
        }

        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 20px;
            border-left: 1px solid #1E1E24;
        }

        QComboBox QAbstractItemView {
            background-color: #0B0B0E;
            color: #FFFFFF;
            border: 1px solid #1E1E24;
            selection-background-color: #FF4400;
            selection-color: #FFFFFF;
        }

        /* Sliders */
        QSlider::groove:horizontal {
            height: 4px;
            background: #141418;
            border: 1px solid #1E1E24;
        }

        QSlider::sub-page:horizontal {
            background: #FF4400;
        }

        QSlider::handle:horizontal {
            background: #FFFFFF;
            border: 1px solid #FFFFFF;
            width: 12px;
            margin-top: -5px;
            margin-bottom: -5px;
        }

        QSlider::handle:horizontal:hover {
            background: #CCFF00;
            border: 1px solid #CCFF00;
        }

        QSlider::groove:vertical {
            width: 4px;
            background: #141418;
            border: 1px solid #1E1E24;
        }

        QSlider::add-page:vertical {
            background: #CCFF00;
        }

        QSlider::handle:vertical {
            background: #FFFFFF;
            border: 1px solid #FFFFFF;
            height: 12px;
            margin-left: -5px;
            margin-right: -5px;
        }

        QSlider::handle:vertical:hover {
            background: #CCFF00;
            border: 1px solid #CCFF00;
        }

        /* Scrollbars */
        QScrollBar:vertical {
            background: #070709;
            width: 8px;
            margin: 0px;
            border-left: 1px solid #1E1E24;
        }

        QScrollBar::handle:vertical {
            background: #1E1E24;
            min-height: 20px;
        }

        QScrollBar::handle:vertical:hover {
            background: #333342;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }

        QScrollBar:horizontal {
            background: #070709;
            height: 8px;
            margin: 0px;
            border-top: 1px solid #1E1E24;
        }

        QScrollBar::handle:horizontal {
            background: #1E1E24;
            min-width: 20px;
        }

        QScrollBar::handle:horizontal:hover {
            background: #333342;
        }

        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }

        /* Table View */
        QTableView, QTableWidget {
            background-color: #070709;
            color: #FFFFFF;
            gridline-color: #1E1E24;
            border: 1px solid #1E1E24;
            selection-background-color: #1E1E24;
            selection-color: #CCFF00;
        }

        QHeaderView::section {
            background-color: #0B0B0E;
            color: #777788;
            border: none;
            border-right: 1px solid #1E1E24;
            border-bottom: 1px solid #1E1E24;
            padding: 4px 8px;
            font-weight: bold;
            font-size: 10px;
            text-transform: uppercase;
        }

        /* Tooltip */
        QToolTip {
            background-color: #0B0B0E;
            color: #FFFFFF;
            border: 1px solid #00E5FF;
            padding: 4px 8px;
            font-family: "JetBrains Mono", "DejaVu Sans Mono", monospace;
            font-size: 10px;
        }

        /* Menu */
        QMenu {
            background-color: #0B0B0E;
            color: #FFFFFF;
            border: 1px solid #1E1E24;
            padding: 2px;
        }

        QMenu::item {
            padding: 6px 20px 6px 15px;
            font-size: 11px;
        }

        QMenu::item:selected {
            background-color: #1E1E24;
            color: #CCFF00;
        }

        QMenu::separator {
            height: 1px;
            background: #1E1E24;
            margin: 2px 0px;
        }
    )");
}

QString BrutalistTheme::primaryButtonStyleSheet()
{
    return QString(R"(
        QPushButton {
            background-color: #0B0B0E;
            color: #FFFFFF;
            border: 1px solid #1E1E24;
            border-radius: 4px;
            padding: 5px 12px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #141418;
            border: 1px solid #333342;
            color: #00E5FF;
        }
        QPushButton:pressed {
            background-color: #1E1E24;
            border: 1px solid #FF4400;
            color: #FF4400;
        }
    )");
}

QString BrutalistTheme::accentOrangeButtonStyleSheet()
{
    return QString(R"(
        QPushButton {
            background-color: #FF4400;
            color: #070709;
            border: 1px solid #FF4400;
            border-radius: 12px;
            padding: 4px 14px;
            font-weight: bold;
            font-size: 10px;
        }
        QPushButton:hover {
            background-color: #FF6622;
            border: 1px solid #FF6622;
            color: #FFFFFF;
        }
        QPushButton:pressed {
            background-color: #CC3300;
            border: 1px solid #CC3300;
        }
    )");
}

QString BrutalistTheme::accentLimeButtonStyleSheet()
{
    return QString(R"(
        QPushButton {
            background-color: #CCFF00;
            color: #070709;
            border: 1px solid #CCFF00;
            border-radius: 12px;
            padding: 4px 14px;
            font-weight: bold;
            font-size: 10px;
        }
        QPushButton:hover {
            background-color: #DDFF33;
            border: 1px solid #DDFF33;
            color: #070709;
        }
        QPushButton:pressed {
            background-color: #99CC00;
            border: 1px solid #99CC00;
        }
    )");
}

QString BrutalistTheme::panelStyleSheet()
{
    return QString(R"(
        background-color: #0B0B0E;
        border: 1px solid #1E1E24;
        border-radius: 6px;
    )");
}

QString BrutalistTheme::tableStyleSheet()
{
    return QString(R"(
        QTableView {
            background-color: #070709;
            color: #FFFFFF;
            gridline-color: #1E1E24;
            border: 1px solid #1E1E24;
            border-radius: 4px;
            selection-background-color: #141418;
            selection-color: #CCFF00;
        }
        QHeaderView::section {
            background-color: #0B0B0E;
            color: #777788;
            border: none;
            border-right: 1px solid #1E1E24;
            border-bottom: 1px solid #1E1E24;
            padding: 5px 8px;
            font-weight: bold;
            font-size: 9px;
            letter-spacing: 0.5px;
        }
    )");
}

QString BrutalistTheme::pillButtonStyleSheet()
{
    return QString(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(26, 26, 34, 0.9), stop:1 rgba(14, 14, 20, 0.95));
            color: #FFFFFF;
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 12px;
            padding: 4px 12px;
            font-weight: bold;
            font-size: 10px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(36, 36, 48, 0.95), stop:1 rgba(20, 20, 28, 0.98));
            border: 1px solid rgba(0, 229, 255, 0.6);
            color: #00E5FF;
        }
        QPushButton:pressed {
            background: #FF4400;
            border: 1px solid #FF4400;
            color: #070709;
        }
        QPushButton:checked {
            background: rgba(255, 68, 0, 0.25);
            border: 1px solid #FF4400;
            color: #FF4400;
        }
    )");
}

QString BrutalistTheme::transportDockPillStyleSheet()
{
    return QString(R"(
        QWidget#TransportDock {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(20, 20, 28, 0.92), stop:1 rgba(10, 10, 14, 0.96));
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 20px;
        }
    )");
}

QString BrutalistTheme::floatingHeaderStyleSheet()
{
    return QString(R"(
        QWidget#FloatingHeader {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(18, 18, 24, 0.90), stop:1 rgba(10, 10, 14, 0.94));
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 12px;
        }
    )");
}

QString BrutalistTheme::searchInputStyleSheet()
{
    return QString(R"(
        QLineEdit {
            background-color: rgba(16, 16, 22, 0.85);
            color: #FFFFFF;
            border: 1px solid rgba(255, 255, 255, 0.10);
            border-radius: 13px;
            padding: 4px 12px;
            font-size: 10px;
            letter-spacing: 0.5px;
        }
        QLineEdit:focus {
            border: 1px solid #00E5FF;
            background-color: rgba(20, 20, 28, 0.95);
        }
    )");
}

QString BrutalistTheme::videoEqualizerDialogStyleSheet()
{
    return QString(R"(
        QDialog {
            background-color: #0B0B0E;
            border: 1px solid #1E1E24;
            border-radius: 8px;
        }
        QLabel {
            color: #E2E2EA;
            font-family: "JetBrains Mono", "DejaVu Sans Mono", monospace;
        }
        QSlider::groove:horizontal {
            height: 4px;
            background: #14141E;
            border: 1px solid #22222E;
            border-radius: 2px;
        }
        QSlider::sub-page:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #CCFF00, stop:1 #00E5FF);
            border-radius: 2px;
        }
        QSlider::handle:horizontal {
            background: #FFFFFF;
            border: 1px solid #FFFFFF;
            width: 12px;
            margin-top: -4px;
            margin-bottom: -4px;
            border-radius: 6px;
        }
        QSlider::handle:horizontal:hover {
            background: #00E5FF;
            border: 1px solid #00E5FF;
        }
    )");
}

void BrutalistTheme::applyTheme(QApplication *app)
{
    if (!app) return;
    app->setStyle(QStyleFactory::create("Fusion"));
    app->setStyleSheet(globalStyleSheet());

    QPalette palette;
    palette.setColor(QPalette::Window, BG_DEEP_OBSIDIAN);
    palette.setColor(QPalette::WindowText, TEXT_HIGH_CONTRAST);
    palette.setColor(QPalette::Base, BG_DEEP_OBSIDIAN);
    palette.setColor(QPalette::AlternateBase, SURFACE_PANEL_BASE);
    palette.setColor(QPalette::ToolTipBase, SURFACE_PANEL_BASE);
    palette.setColor(QPalette::ToolTipText, TEXT_HIGH_CONTRAST);
    palette.setColor(QPalette::Text, TEXT_HIGH_CONTRAST);
    palette.setColor(QPalette::Button, SURFACE_PANEL_BASE);
    palette.setColor(QPalette::ButtonText, TEXT_HIGH_CONTRAST);
    palette.setColor(QPalette::BrightText, ACCENT_SAFETY_ORANGE);
    palette.setColor(QPalette::Highlight, ACCENT_SAFETY_ORANGE);
    palette.setColor(QPalette::HighlightedText, TEXT_HIGH_CONTRAST);
    app->setPalette(palette);
}

void BrutalistTheme::applyTheme(QWidget *widget)
{
    if (!widget) return;
    widget->setStyleSheet(globalStyleSheet());
}

} // namespace UI
} // namespace Penguin
