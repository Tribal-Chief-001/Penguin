#ifndef BRUTALISTTHEME_H
#define BRUTALISTTHEME_H

#include <QColor>
#include <QFont>
#include <QString>
#include <QWidget>
#include <QApplication>

namespace Penguin {
namespace UI {

class BrutalistTheme {
public:
    // Color Palette Tokens
    static const QColor BG_DEEP_OBSIDIAN;       // #070709
    static const QColor SURFACE_PANEL_BASE;     // #0B0B0E
    static const QColor SURFACE_RAISED;         // #141418
    static const QColor GRID_STRUCTURAL_BORDER; // #1E1E24
    static const QColor GRID_LINE_ACTIVE;       // #333342
    static const QColor TEXT_HIGH_CONTRAST;     // #FFFFFF
    static const QColor TEXT_SECONDARY_DIM;     // #777788
    static const QColor TEXT_MUTED;             // #444455
    static const QColor ACCENT_SAFETY_ORANGE;   // #FF4400
    static const QColor ACCENT_SIGNAL_LIME;     // #CCFF00
    static const QColor ACCENT_TELEMETRY_CYAN;  // #00E5FF
    static const QColor ACCENT_CLIP_RED;        // #FF2200

    // Typography
    static QFont monospaceFont(int pointSize = 10, QFont::Weight weight = QFont::Normal);
    static QFont sansFont(int pointSize = 10, QFont::Weight weight = QFont::Normal);

    // QSS Stylesheets
    static QString globalStyleSheet();
    static QString primaryButtonStyleSheet();
    static QString accentOrangeButtonStyleSheet();
    static QString accentLimeButtonStyleSheet();
    static QString panelStyleSheet();
    static QString tableStyleSheet();

    // Theme initialization
    static void applyTheme(QApplication *app);
    static void applyTheme(QWidget *widget);
};

} // namespace UI
} // namespace Penguin

#endif // BRUTALISTTHEME_H
