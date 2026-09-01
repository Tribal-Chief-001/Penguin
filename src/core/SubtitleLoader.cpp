#include "SubtitleLoader.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <algorithm>

namespace Penguin {
namespace Core {

SubtitleLoader::SubtitleLoader()
{
}

void SubtitleLoader::clear()
{
    m_cues.clear();
    m_format = SubtitleFormat::Unknown;
    m_filePath.clear();
}

QString SubtitleLoader::formatToString(SubtitleFormat format)
{
    switch (format) {
    case SubtitleFormat::SRT: return "SRT";
    case SubtitleFormat::ASS: return "ASS";
    case SubtitleFormat::VTT: return "WebVTT";
    case SubtitleFormat::Unknown:
    default:
        return "Unknown";
    }
}

SubtitleFormat SubtitleLoader::detectFormat(const QString &filePath, const QString &content)
{
    if (!filePath.isEmpty()) {
        QString ext = QFileInfo(filePath).suffix().toLower();
        if (ext == "srt") return SubtitleFormat::SRT;
        if (ext == "ass" || ext == "ssa") return SubtitleFormat::ASS;
        if (ext == "vtt") return SubtitleFormat::VTT;
    }

    if (!content.isEmpty()) {
        QString trimmed = content.trimmed();
        if (trimmed.startsWith("WEBVTT", Qt::CaseInsensitive)) {
            return SubtitleFormat::VTT;
        }
        if (trimmed.contains("[Script Info]", Qt::CaseInsensitive) ||
            trimmed.contains("[Events]", Qt::CaseInsensitive)) {
            return SubtitleFormat::ASS;
        }
        // Check for SRT pattern (1\n00:00:...)
        static const QRegularExpression srtRegex("^\\d+\\s+[\\r\\n]+\\d{1,2}:\\d{2}:\\d{2}");
        if (srtRegex.match(trimmed).hasMatch()) {
            return SubtitleFormat::SRT;
        }
    }

    return SubtitleFormat::Unknown;
}

bool SubtitleLoader::loadFromFile(const QString &filePath)
{
    clear();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    m_filePath = filePath;
    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("UTF-8");
#endif
    QString content = in.readAll();
    file.close();

    m_format = detectFormat(filePath, content);
    return parseContent(content, m_format);
}

bool SubtitleLoader::parseContent(const QString &content, SubtitleFormat format)
{
    if (format == SubtitleFormat::Unknown) {
        format = detectFormat(m_filePath, content);
    }
    m_format = format;

    switch (format) {
    case SubtitleFormat::SRT:
        return parseSrt(content);
    case SubtitleFormat::ASS:
        return parseAss(content);
    case SubtitleFormat::VTT:
        return parseVtt(content);
    default:
        // Try SRT first, then VTT, then ASS
        if (parseSrt(content)) {
            m_format = SubtitleFormat::SRT;
            return true;
        }
        if (parseVtt(content)) {
            m_format = SubtitleFormat::VTT;
            return true;
        }
        if (parseAss(content)) {
            m_format = SubtitleFormat::ASS;
            return true;
        }
        return false;
    }
}

qint64 SubtitleLoader::parseSrtTimestamp(const QString &ts)
{
    // 00:01:20,000 or 00:01:20.000 or 01:20.000
    QString clean = ts.trimmed();
    clean.replace(',', '.');
    QStringList parts = clean.split(':');
    if (parts.size() >= 3) {
        qint64 hh = parts[0].toLongLong();
        qint64 mm = parts[1].toLongLong();
        QStringList secParts = parts[2].split('.');
        qint64 ss = secParts[0].toLongLong();
        qint64 ms = 0;
        if (secParts.size() >= 2) {
            QString frac = secParts[1];
            if (frac.length() == 3) ms = frac.toLongLong();
            else if (frac.length() == 2) ms = frac.toLongLong() * 10;
            else if (frac.length() == 1) ms = frac.toLongLong() * 100;
            else ms = frac.left(3).toLongLong();
        }
        return (hh * 3600 + mm * 60 + ss) * 1000 + ms;
    } else if (parts.size() == 2) {
        qint64 mm = parts[0].toLongLong();
        QStringList secParts = parts[1].split('.');
        qint64 ss = secParts[0].toLongLong();
        qint64 ms = 0;
        if (secParts.size() >= 2) {
            QString frac = secParts[1];
            if (frac.length() == 3) ms = frac.toLongLong();
            else if (frac.length() == 2) ms = frac.toLongLong() * 10;
            else if (frac.length() == 1) ms = frac.toLongLong() * 100;
            else ms = frac.left(3).toLongLong();
        }
        return (mm * 60 + ss) * 1000 + ms;
    }
    return 0;
}

qint64 SubtitleLoader::parseAssTimestamp(const QString &ts)
{
    // 0:01:20.00
    QString clean = ts.trimmed();
    QStringList parts = clean.split(':');
    if (parts.size() >= 3) {
        qint64 hh = parts[0].toLongLong();
        qint64 mm = parts[1].toLongLong();
        QStringList secParts = parts[2].split('.');
        qint64 ss = secParts[0].toLongLong();
        qint64 ms = 0;
        if (secParts.size() >= 2) {
            QString frac = secParts[1];
            if (frac.length() == 2) ms = frac.toLongLong() * 10;
            else if (frac.length() == 3) ms = frac.toLongLong();
            else ms = frac.left(3).toLongLong();
        }
        return (hh * 3600 + mm * 60 + ss) * 1000 + ms;
    }
    return 0;
}

QString SubtitleLoader::stripFormatting(const QString &rawText)
{
    QString clean = rawText;
    // Strip ASS override tags {\...}
    static const QRegularExpression assTagRegex("\\{[^}]*\\}");
    clean.remove(assTagRegex);
    // Strip HTML tags <b>, <i>, <font...>, </...>
    static const QRegularExpression htmlTagRegex("<[^>]*>");
    clean.remove(htmlTagRegex);
    // Replace ASS hard line breaks \N, \n, \h
    clean.replace("\\N", "\n");
    clean.replace("\\n", "\n");
    clean.replace("\\h", " ");
    return clean.trimmed();
}

bool SubtitleLoader::parseSrt(const QString &content)
{
    m_cues.clear();
    QStringList blocks = content.split(QRegularExpression("[\\r\\n]{2,}"), Qt::SkipEmptyParts);
    static const QRegularExpression timeArrowRegex("(\\d{1,2}:\\d{2}:\\d{2}[,\\.]\\d{2,3})\\s*-->\\s*(\\d{1,2}:\\d{2}:\\d{2}[,\\.]\\d{2,3})");

    int cueIndex = 1;
    for (const QString &block : blocks) {
        QStringList lines = block.split(QRegularExpression("[\\r\\n]+"), Qt::SkipEmptyParts);
        if (lines.isEmpty()) continue;

        int timeLineIdx = -1;
        QRegularExpressionMatch timeMatch;
        for (int i = 0; i < lines.size(); ++i) {
            timeMatch = timeArrowRegex.match(lines[i]);
            if (timeMatch.hasMatch()) {
                timeLineIdx = i;
                break;
            }
        }

        if (timeLineIdx >= 0) {
            SubtitleCue cue;
            cue.index = cueIndex++;
            cue.startMs = parseSrtTimestamp(timeMatch.captured(1));
            cue.endMs = parseSrtTimestamp(timeMatch.captured(2));

            QStringList textLines;
            for (int i = timeLineIdx + 1; i < lines.size(); ++i) {
                textLines.append(lines[i]);
            }
            cue.text = textLines.join('\n');
            cue.plainText = stripFormatting(cue.text);

            if (cue.endMs > cue.startMs && !cue.plainText.isEmpty()) {
                m_cues.append(cue);
            }
        }
    }

    std::stable_sort(m_cues.begin(), m_cues.end());
    return !m_cues.isEmpty();
}

bool SubtitleLoader::parseVtt(const QString &content)
{
    m_cues.clear();
    QStringList blocks = content.split(QRegularExpression("[\\r\\n]{2,}"), Qt::SkipEmptyParts);
    static const QRegularExpression timeArrowRegex("(?:(\\d{1,2}:)?\\d{2}:\\d{2}[\\.]\\d{2,3})\\s*-->\\s*(?:(\\d{1,2}:)?\\d{2}:\\d{2}[\\.]\\d{2,3})");

    int cueIndex = 1;
    for (const QString &block : blocks) {
        QStringList lines = block.split(QRegularExpression("[\\r\\n]+"), Qt::SkipEmptyParts);
        if (lines.isEmpty()) continue;

        int timeLineIdx = -1;
        QRegularExpressionMatch timeMatch;
        for (int i = 0; i < lines.size(); ++i) {
            timeMatch = timeArrowRegex.match(lines[i]);
            if (timeMatch.hasMatch()) {
                timeLineIdx = i;
                break;
            }
        }

        if (timeLineIdx >= 0) {
            SubtitleCue cue;
            cue.index = cueIndex++;
            cue.startMs = parseSrtTimestamp(timeMatch.captured(0).split("-->")[0]);
            cue.endMs = parseSrtTimestamp(timeMatch.captured(0).split("-->")[1]);

            QStringList textLines;
            for (int i = timeLineIdx + 1; i < lines.size(); ++i) {
                textLines.append(lines[i]);
            }
            cue.text = textLines.join('\n');
            cue.plainText = stripFormatting(cue.text);

            if (cue.endMs > cue.startMs && !cue.plainText.isEmpty()) {
                m_cues.append(cue);
            }
        }
    }

    std::stable_sort(m_cues.begin(), m_cues.end());
    return !m_cues.isEmpty();
}

bool SubtitleLoader::parseAss(const QString &content)
{
    m_cues.clear();
    QStringList lines = content.split(QRegularExpression("[\\r\\n]+"), Qt::SkipEmptyParts);

    bool inEvents = false;
    QStringList formatFields;
    int startIndex = 1;
    int endIndex = 2;
    int styleIndex = 3;
    int textIndex = 9;
    int cueIndex = 1;

    for (const QString &rawLine : lines) {
        QString line = rawLine.trimmed();
        if (line.startsWith("[Events]", Qt::CaseInsensitive)) {
            inEvents = true;
            continue;
        }
        if (line.startsWith("[") && inEvents) {
            inEvents = false;
            continue;
        }

        if (inEvents) {
            if (line.startsWith("Format:", Qt::CaseInsensitive)) {
                QString fieldsStr = line.mid(7).trimmed();
                formatFields = fieldsStr.split(QRegularExpression("\\s*,\\s*"));
                for (int i = 0; i < formatFields.size(); ++i) {
                    QString f = formatFields[i].toLower();
                    if (f == "start") startIndex = i;
                    else if (f == "end") endIndex = i;
                    else if (f == "style") styleIndex = i;
                    else if (f == "text") textIndex = i;
                }
            } else if (line.startsWith("Dialogue:", Qt::CaseInsensitive)) {
                QString data = line.mid(9).trimmed();
                // ASS dialogue line has comma-separated fields, with Text field being the last (may contain commas)
                QStringList parts;
                int startPos = 0;
                for (int f = 0; f < formatFields.size() - 1; ++f) {
                    int commaPos = data.indexOf(',', startPos);
                    if (commaPos == -1) break;
                    parts.append(data.mid(startPos, commaPos - startPos).trimmed());
                    startPos = commaPos + 1;
                }
                parts.append(data.mid(startPos).trimmed()); // Text field

                if (parts.size() > std::max(startIndex, std::max(endIndex, textIndex))) {
                    SubtitleCue cue;
                    cue.index = cueIndex++;
                    cue.startMs = parseAssTimestamp(parts[startIndex]);
                    cue.endMs = parseAssTimestamp(parts[endIndex]);
                    if (styleIndex < parts.size()) cue.style = parts[styleIndex];
                    cue.text = parts[textIndex];
                    cue.plainText = stripFormatting(cue.text);

                    if (cue.endMs > cue.startMs && !cue.plainText.isEmpty()) {
                        m_cues.append(cue);
                    }
                }
            }
        }
    }

    std::stable_sort(m_cues.begin(), m_cues.end());
    return !m_cues.isEmpty();
}

QString SubtitleLoader::activeSubtitleText(qint64 positionMs) const
{
    auto active = activeCues(positionMs);
    if (active.isEmpty()) return QString();

    QStringList lines;
    for (const auto &cue : active) {
        lines.append(cue.plainText);
    }
    return lines.join('\n');
}

QList<SubtitleCue> SubtitleLoader::activeCues(qint64 positionMs) const
{
    QList<SubtitleCue> result;
    for (const auto &cue : m_cues) {
        if (positionMs >= cue.startMs && positionMs < cue.endMs) {
            result.append(cue);
        } else if (cue.startMs > positionMs) {
            // Because cues are sorted by startMs, once startMs exceeds position, can stop if all cues are non-overlapping or keep searching
            // For overlapping cues, keep checking
        }
    }
    return result;
}

} // namespace Core
} // namespace Penguin
