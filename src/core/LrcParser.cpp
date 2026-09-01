#include "LrcParser.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <algorithm>

namespace Penguin {
namespace Core {

LrcParser::LrcParser()
    : m_offsetMs(0)
{
}

void LrcParser::clear()
{
    m_cues.clear();
    m_title.clear();
    m_artist.clear();
    m_album.clear();
    m_author.clear();
    m_offsetMs = 0;
}

bool LrcParser::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("UTF-8");
#endif
    QString content = in.readAll();
    file.close();

    return parse(content);
}

qint64 LrcParser::parseTimestamp(const QString &ts)
{
    // Matches mm:ss.xx or mm:ss.xxx or mm:ss:xx
    QString clean = ts.trimmed();
    clean.replace(':', '.');
    QStringList parts = clean.split('.');

    if (parts.size() >= 2) {
        qint64 minutes = parts[0].toLongLong();
        qint64 seconds = parts[1].toLongLong();
        qint64 ms = 0;

        if (parts.size() >= 3) {
            QString fracStr = parts[2];
            if (fracStr.length() == 2) {
                // Centiseconds (1/100s)
                ms = fracStr.toLongLong() * 10;
            } else if (fracStr.length() == 3) {
                // Milliseconds
                ms = fracStr.toLongLong();
            } else if (fracStr.length() == 1) {
                ms = fracStr.toLongLong() * 100;
            } else {
                ms = fracStr.left(3).toLongLong();
            }
        }

        return (minutes * 60 + seconds) * 1000 + ms;
    }

    return 0;
}

bool LrcParser::parse(const QString &lrcContent)
{
    clear();
    if (lrcContent.trimmed().isEmpty()) {
        return false;
    }

    // Regex for metadata tags: [ar:Artist], [ti:Title], [al:Album], [by:Author], [offset:+/-ms]
    static const QRegularExpression metaRegex("^\\[(ar|ti|al|by|offset|length):([^\\]]*)\\]", QRegularExpression::CaseInsensitiveOption);
    // Regex for timestamps: [00:12.34], [00:12:34], [00:12.345], [00:12.3], [00:12], [120:30.00]
    static const QRegularExpression timeRegex("\\[(\\d{1,4}:\\d{2}(?:[\\.:]\\d{1,3})?)\\]");

    QStringList lines = lrcContent.split(QRegularExpression("[\\r\\n]+"), Qt::SkipEmptyParts);
    QList<LyricCue> parsedCues;

    for (const QString &rawLine : lines) {
        QString line = rawLine.trimmed();
        if (line.isEmpty()) continue;

        // Check for metadata tag
        auto metaMatch = metaRegex.match(line);
        if (metaMatch.hasMatch()) {
            QString tag = metaMatch.captured(1).toLower();
            QString val = metaMatch.captured(2).trimmed();
            if (tag == "ti") m_title = val;
            else if (tag == "ar") m_artist = val;
            else if (tag == "al") m_album = val;
            else if (tag == "by") m_author = val;
            else if (tag == "offset") m_offsetMs = val.toLongLong();
            continue;
        }

        // Collect all timestamps in this line
        QList<qint64> timestamps;
        int lastMatchEnd = 0;
        auto timeIterator = timeRegex.globalMatch(line);
        while (timeIterator.hasNext()) {
            auto match = timeIterator.next();
            timestamps.append(parseTimestamp(match.captured(1)));
            lastMatchEnd = match.capturedEnd();
        }

        if (!timestamps.isEmpty()) {
            QString lyricText = line.mid(lastMatchEnd).trimmed();
            for (qint64 ts : timestamps) {
                LyricCue cue;
                cue.timestampMs = ts;
                cue.text = lyricText;
                parsedCues.append(cue);
            }
        }
    }

    // Apply global offset
    if (m_offsetMs != 0) {
        for (auto &cue : parsedCues) {
            cue.timestampMs = std::max(0LL, cue.timestampMs + m_offsetMs);
        }
    }

    // Sort cues chronologically
    std::stable_sort(parsedCues.begin(), parsedCues.end());
    m_cues = parsedCues;

    return !m_cues.isEmpty() || !m_title.isEmpty() || !m_artist.isEmpty();
}

int LrcParser::findActiveCueIndex(qint64 positionMs) const
{
    if (m_cues.isEmpty()) return -1;
    if (positionMs < m_cues.first().timestampMs) return -1;
    if (positionMs >= m_cues.last().timestampMs) return static_cast<int>(m_cues.size() - 1);

    // Binary search (std::upper_bound)
    auto it = std::upper_bound(m_cues.begin(), m_cues.end(), positionMs,
                               [](qint64 pos, const LyricCue &cue) {
                                   return pos < cue.timestampMs;
                               });

    int index = static_cast<int>(std::distance(m_cues.begin(), it) - 1);
    int maxIndex = static_cast<int>(m_cues.size() - 1);
    return std::max(0, std::min(index, maxIndex));
}

LyricCue LrcParser::activeCue(qint64 positionMs) const
{
    int idx = findActiveCueIndex(positionMs);
    if (idx >= 0 && idx < m_cues.size()) {
        return m_cues[idx];
    }
    return LyricCue{-1, QString()};
}

qint64 LrcParser::cueTimestampAt(int index) const
{
    if (index >= 0 && index < m_cues.size()) {
        return m_cues[index].timestampMs;
    }
    return 0;
}

QString LrcParser::cueTextAt(int index) const
{
    if (index >= 0 && index < m_cues.size()) {
        return m_cues[index].text;
    }
    return QString();
}

} // namespace Core
} // namespace Penguin
