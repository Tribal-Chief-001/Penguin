#ifndef SUBTITLELOADER_H
#define SUBTITLELOADER_H

#include <QString>
#include <QList>
#include <QtGlobal>

namespace Penguin {
namespace Core {

enum class SubtitleFormat {
    Unknown,
    SRT,
    ASS,
    VTT
};

struct SubtitleCue {
    int index = 0;
    qint64 startMs = 0;
    qint64 endMs = 0;
    QString text;
    QString style;
    QString plainText; // With formatting tags stripped

    bool operator<(const SubtitleCue &other) const {
        if (startMs != other.startMs) return startMs < other.startMs;
        return endMs < other.endMs;
    }
};

class SubtitleLoader {
public:
    SubtitleLoader();

    // Load subtitle from local file (.srt, .ass, .ssa, .vtt)
    bool loadFromFile(const QString &filePath);

    // Parse subtitle string content
    bool parseContent(const QString &content, SubtitleFormat format = SubtitleFormat::Unknown);

    // Format specific parsers
    bool parseSrt(const QString &content);
    bool parseAss(const QString &content);
    bool parseVtt(const QString &content);

    // Format detection
    static SubtitleFormat detectFormat(const QString &filePath, const QString &content = QString());
    static QString formatToString(SubtitleFormat format);

    // Query active subtitle text for a given millisecond position
    QString activeSubtitleText(qint64 positionMs) const;
    QList<SubtitleCue> activeCues(qint64 positionMs) const;

    // Subtitle data accessors
    const QList<SubtitleCue>& cues() const { return m_cues; }
    int cueCount() const { return m_cues.size(); }
    bool isValid() const { return !m_cues.isEmpty(); }
    SubtitleFormat format() const { return m_format; }
    QString formatName() const { return formatToString(m_format); }
    QString filePath() const { return m_filePath; }

    void clear();

    // Utility: strip formatting tags (HTML/ASS override tags)
    static QString stripFormatting(const QString &rawText);

private:
    static qint64 parseSrtTimestamp(const QString &ts);
    static qint64 parseAssTimestamp(const QString &ts);

    QList<SubtitleCue> m_cues;
    SubtitleFormat m_format = SubtitleFormat::Unknown;
    QString m_filePath;
};

} // namespace Core
} // namespace Penguin

#endif // SUBTITLELOADER_H
