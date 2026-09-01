#ifndef TIMECODEFORMATTER_H
#define TIMECODEFORMATTER_H

#include <QString>
#include <QtGlobal>

namespace Penguin {
namespace Core {

class TimecodeFormatter {
public:
    // Format millisecond timestamp into SMPTE timecode (HH:MM:SS:FF or HH:MM:SS;FF)
    static QString formatTimecode(qint64 positionMs, double fps = 30.0, bool dropFrame = false);

    // Format remaining time into -HH:MM:SS:FF
    static QString formatRemaining(qint64 positionMs, qint64 durationMs, double fps = 30.0, bool dropFrame = false);

    // Format millisecond timestamp into standard display MM:SS or HH:MM:SS
    static QString formatSimpleTime(qint64 positionMs);

    // Parse SMPTE timecode string (HH:MM:SS:FF or HH:MM:SS;FF or MM:SS.xx) to milliseconds
    static qint64 timecodeToMs(const QString &timecode, double fps = 30.0);

    // Frame to millisecond conversion
    static qint64 framesToMs(qint64 frames, double fps = 30.0);

    // Millisecond to frame number conversion
    static qint64 msToFrames(qint64 positionMs, double fps = 30.0);

    // Check if a frame rate is standard NTSC drop-frame (e.g. 29.97, 59.94)
    static bool isDropFrameRate(double fps);

    // Get nearest standard integer FPS
    static int nominalFps(double fps);

    // Validate if a string is a valid SMPTE timecode
    static bool isValidTimecode(const QString &timecode);
};

} // namespace Core
} // namespace Penguin

#endif // TIMECODEFORMATTER_H
