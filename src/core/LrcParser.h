#ifndef LRCPARSER_H
#define LRCPARSER_H

#include <QString>
#include <QList>
#include <QtGlobal>

namespace Penguin {
namespace Core {

struct LyricCue {
    qint64 timestampMs = 0; // Milliseconds
    QString text;

    bool operator<(const LyricCue &other) const {
        return timestampMs < other.timestampMs;
    }
};

class LrcParser {
public:
    LrcParser();

    // Parse raw LRC string content
    bool parse(const QString &lrcContent);

    // Load and parse LRC from local file
    bool loadFromFile(const QString &filePath);

    // Clear all parsed data
    void clear();

    // List of chronologically sorted lyric cues
    const QList<LyricCue>& cues() const { return m_cues; }
    int count() const { return m_cues.size(); }
    bool isEmpty() const { return m_cues.isEmpty(); }

    // Binary search for active lyric cue at current playback position (ms)
    // Returns index in cues() or -1 if before the first cue
    int findActiveCueIndex(qint64 positionMs) const;

    // Get active lyric cue at current playback position
    LyricCue activeCue(qint64 positionMs) const;

    // Get timestamp of cue at given index (for click-to-seek)
    qint64 cueTimestampAt(int index) const;

    // Get text of cue at given index
    QString cueTextAt(int index) const;

    // Metadata tags
    QString title() const { return m_title; }
    QString artist() const { return m_artist; }
    QString album() const { return m_album; }
    QString author() const { return m_author; }
    qint64 offsetMs() const { return m_offsetMs; }

private:
    static qint64 parseTimestamp(const QString &timestampStr);

    QList<LyricCue> m_cues;
    QString m_title;
    QString m_artist;
    QString m_album;
    QString m_author;
    qint64 m_offsetMs = 0;
};

} // namespace Core
} // namespace Penguin

#endif // LRCPARSER_H
