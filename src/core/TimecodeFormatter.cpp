#include "TimecodeFormatter.h"
#include <cmath>
#include <algorithm>
#include <QStringList>
#include <QRegularExpression>

namespace Penguin {
namespace Core {

int TimecodeFormatter::nominalFps(double fps)
{
    if (fps <= 0.0) return 30;
    return static_cast<int>(std::round(fps));
}

bool TimecodeFormatter::isDropFrameRate(double fps)
{
    // Common drop frame rates: 29.97 (30000/1001) and 59.94 (60000/1001)
    return (std::abs(fps - 29.97) < 0.01) || (std::abs(fps - (30000.0 / 1001.0)) < 0.001) ||
           (std::abs(fps - 59.94) < 0.01) || (std::abs(fps - (60000.0 / 1001.0)) < 0.001);
}

QString TimecodeFormatter::formatTimecode(qint64 positionMs, double fps, bool dropFrame)
{
    if (fps <= 0.0) fps = 30.0;
    bool isNegative = positionMs < 0;
    qint64 absMs = std::abs(positionMs);
    double seconds = absMs / 1000.0;

    int fpsInt = nominalFps(fps);
    if (fpsInt <= 0) fpsInt = 30;

    if (dropFrame && isDropFrameRate(fps)) {
        // Drop-frame calculation (SMPTE 12M standard)
        int dropFramesPerMinute = (fpsInt == 60) ? 4 : 2;
        int framesPer10Min = (fpsInt == 60) ? 35964 : 17982;
        int framesPerMinute0 = fpsInt * 60; // First minute of each 10-minute block has full frames
        int framesPerMinuteRem = fpsInt * 60 - dropFramesPerMinute;

        qint64 totalFrames = static_cast<qint64>(std::floor(seconds * fps + 0.5));
        
        qint64 d = totalFrames / framesPer10Min;
        qint64 m = totalFrames % framesPer10Min;

        qint64 adjustedFrames = totalFrames + (dropFramesPerMinute * 9 * d);
        if (m >= framesPerMinute0) {
            adjustedFrames += dropFramesPerMinute * (1 + (m - framesPerMinute0) / framesPerMinuteRem);
        }

        qint64 ff = adjustedFrames % fpsInt;
        qint64 ss = (adjustedFrames / fpsInt) % 60;
        qint64 mm = (adjustedFrames / (fpsInt * 60)) % 60;
        qint64 hh = adjustedFrames / (fpsInt * 3600);

        QString sign = isNegative ? "-" : "";
        return QString("%1%2:%3:%4;%5")
            .arg(sign)
            .arg(hh, 2, 10, QChar('0'))
            .arg(mm, 2, 10, QChar('0'))
            .arg(ss, 2, 10, QChar('0'))
            .arg(ff, 2, 10, QChar('0'));
    } else {
        // Non-drop frame calculation
        qint64 totalFrames = static_cast<qint64>(std::floor(seconds * fps + 0.5));
        qint64 ff = totalFrames % fpsInt;
        qint64 ss = (totalFrames / fpsInt) % 60;
        qint64 mm = (totalFrames / (fpsInt * 60)) % 60;
        qint64 hh = totalFrames / (fpsInt * 3600);

        QString sign = isNegative ? "-" : "";
        return QString("%1%2:%3:%4:%5")
            .arg(sign)
            .arg(hh, 2, 10, QChar('0'))
            .arg(mm, 2, 10, QChar('0'))
            .arg(ss, 2, 10, QChar('0'))
            .arg(ff, 2, 10, QChar('0'));
    }
}

QString TimecodeFormatter::formatRemaining(qint64 positionMs, qint64 durationMs, double fps, bool dropFrame)
{
    qint64 remainingMs = std::max(0LL, durationMs - positionMs);
    QString formatted = formatTimecode(remainingMs, fps, dropFrame);
    return "-" + formatted;
}

QString TimecodeFormatter::formatSimpleTime(qint64 positionMs)
{
    bool isNegative = positionMs < 0;
    qint64 totalSeconds = std::abs(positionMs) / 1000;
    qint64 ss = totalSeconds % 60;
    qint64 mm = (totalSeconds / 60) % 60;
    qint64 hh = totalSeconds / 3600;

    QString sign = isNegative ? "-" : "";
    if (hh > 0) {
        return QString("%1%2:%3:%4")
            .arg(sign)
            .arg(hh, 2, 10, QChar('0'))
            .arg(mm, 2, 10, QChar('0'))
            .arg(ss, 2, 10, QChar('0'));
    } else {
        return QString("%1%2:%3")
            .arg(sign)
            .arg(mm, 2, 10, QChar('0'))
            .arg(ss, 2, 10, QChar('0'));
    }
}

qint64 TimecodeFormatter::framesToMs(qint64 frames, double fps)
{
    if (fps <= 0.0) fps = 30.0;
    return static_cast<qint64>(std::round((frames * 1000.0) / fps));
}

qint64 TimecodeFormatter::msToFrames(qint64 positionMs, double fps)
{
    if (fps <= 0.0) fps = 30.0;
    return static_cast<qint64>(std::floor((positionMs / 1000.0) * fps + 0.5));
}

qint64 TimecodeFormatter::timecodeToMs(const QString &timecode, double fps)
{
    if (fps <= 0.0) fps = 30.0;
    int fpsInt = nominalFps(fps);
    if (fpsInt <= 0) fpsInt = 30;

    QString clean = timecode.trimmed();
    bool isNegative = clean.startsWith('-');
    if (isNegative) clean = clean.mid(1);

    bool isDropFrame = clean.contains(';');
    clean.replace(';', ':');
    clean.replace('.', ':');

    QStringList parts = clean.split(':');
    if (parts.size() == 4) {
        // HH:MM:SS:FF
        qint64 hh = parts[0].toLongLong();
        qint64 mm = parts[1].toLongLong();
        qint64 ss = parts[2].toLongLong();
        qint64 ff = parts[3].toLongLong();

        qint64 totalFrames = 0;
        if (isDropFrame && isDropFrameRate(fps)) {
            int dropFramesPerMinute = (fpsInt == 60) ? 4 : 2;
            qint64 totalMinutes = 60 * hh + mm;
            qint64 dropCount = dropFramesPerMinute * (totalMinutes - (totalMinutes / 10));
            totalFrames = (hh * 3600 + mm * 60 + ss) * fpsInt + ff - dropCount;
        } else {
            totalFrames = (hh * 3600 + mm * 60 + ss) * fpsInt + ff;
        }
        qint64 ms = framesToMs(totalFrames, fps);
        return isNegative ? -ms : ms;
    } else if (parts.size() == 3) {
        // HH:MM:SS or MM:SS:FF
        qint64 p1 = parts[0].toLongLong();
        qint64 p2 = parts[1].toLongLong();
        qint64 p3 = parts[2].toLongLong();
        qint64 ms = (p1 * 3600 + p2 * 60 + p3) * 1000;
        return isNegative ? -ms : ms;
    } else if (parts.size() == 2) {
        // MM:SS
        qint64 mm = parts[0].toLongLong();
        qint64 ss = parts[1].toLongLong();
        qint64 ms = (mm * 60 + ss) * 1000;
        return isNegative ? -ms : ms;
    }

    return 0;
}

bool TimecodeFormatter::isValidTimecode(const QString &timecode)
{
    static const QRegularExpression regex("^-?\\d{1,}:\\d{2}:\\d{2}[:;]\\d{2}$");
    return regex.match(timecode.trimmed()).hasMatch();
}

} // namespace Core
} // namespace Penguin
