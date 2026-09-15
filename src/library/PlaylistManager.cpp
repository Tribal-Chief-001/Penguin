#include "PlaylistManager.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QUrl>
#include <QRandomGenerator>
#include <algorithm>

namespace Penguin {
namespace Library {

PlaylistManager::PlaylistManager(QObject *parent)
    : QObject(parent)
{
}

QStringList PlaylistManager::supportedAudioExtensions()
{
    return {"mp3", "flac", "wav", "opus", "ogg", "m4a", "aac", "alac", "aiff", "wma"};
}

QStringList PlaylistManager::supportedVideoExtensions()
{
    return {"mp4", "mkv", "webm", "avi", "mov", "ts", "flv", "wmv", "m4v"};
}

QStringList PlaylistManager::supportedSubtitleExtensions()
{
    return {"srt", "ass", "ssa", "vtt", "lrc"};
}

bool PlaylistManager::isSupportedMedia(const QString &filePath)
{
    if (filePath.startsWith("http://", Qt::CaseInsensitive) ||
        filePath.startsWith("https://", Qt::CaseInsensitive) ||
        filePath.startsWith("rtmp://", Qt::CaseInsensitive) ||
        filePath.startsWith("ytdl://", Qt::CaseInsensitive)) {
        return true;
    }
    QString ext = QFileInfo(filePath).suffix().toLower();
    return supportedAudioExtensions().contains(ext) || supportedVideoExtensions().contains(ext);
}

UI::PlaylistItem PlaylistManager::itemAt(int index) const
{
    if (index >= 0 && index < m_items.size()) {
        return m_items[index];
    }
    return UI::PlaylistItem();
}

UI::PlaylistItem PlaylistManager::currentItem() const
{
    return itemAt(m_currentIndex);
}

bool PlaylistManager::hasNext() const
{
    if (m_items.isEmpty()) return false;
    if (m_loopMode != LoopMode::None) return true;
    if (m_shuffle) {
        int pos = m_shuffleOrder.indexOf(m_currentIndex);
        return pos >= 0 && pos + 1 < m_shuffleOrder.size();
    }
    return m_currentIndex + 1 < m_items.size();
}

bool PlaylistManager::hasPrevious() const
{
    if (m_items.isEmpty()) return false;
    if (m_loopMode != LoopMode::None) return true;
    if (m_shuffle) {
        int pos = m_shuffleOrder.indexOf(m_currentIndex);
        return pos > 0;
    }
    return m_currentIndex > 0;
}

void PlaylistManager::setCurrentIndex(int index)
{
    if (index < -1 || index >= m_items.size()) return;
    if (m_currentIndex != index) {
        m_currentIndex = index;
        if (m_currentIndex >= 0 && m_currentIndex < m_items.size()) {
            emit currentTrackChanged(m_currentIndex, m_items[m_currentIndex]);
        }
    }
}

void PlaylistManager::addFile(const QString &filePath)
{
    if (filePath.isEmpty()) return;
    bool isUrl = filePath.startsWith("http://", Qt::CaseInsensitive) ||
                 filePath.startsWith("https://", Qt::CaseInsensitive) ||
                 filePath.startsWith("rtmp://", Qt::CaseInsensitive) ||
                 filePath.startsWith("ytdl://", Qt::CaseInsensitive);

    if (!isUrl) {
        QFileInfo fi(filePath);
        if (!fi.exists() || !isSupportedMedia(filePath)) return;

        UI::PlaylistItem item;
        item.filePath = filePath;
        item.title = fi.baseName();
        item.format = fi.suffix().toUpper();

        m_items.append(item);
    } else {
        UI::PlaylistItem item;
        item.filePath = filePath;
        item.title = filePath;
        item.format = "STREAM";
        m_items.append(item);
    }
    rebuildShuffleOrder();
    emit playlistUpdated();
}

void PlaylistManager::addItem(const UI::PlaylistItem &item)
{
    m_items.append(item);
    rebuildShuffleOrder();
    emit playlistUpdated();
}

void PlaylistManager::addItem(const QString &filePath, const QString &title,
                              const QString &artist, const QString &album,
                              qint64 durationMs, const QString &format)
{
    UI::PlaylistItem item;
    item.filePath = filePath;
    item.title = title.isEmpty() ? (filePath.contains("://") ? filePath : QFileInfo(filePath).fileName()) : title;
    item.artist = artist.isEmpty() ? "UNKNOWN ARTIST" : artist;
    item.album = album.isEmpty() ? "UNKNOWN ALBUM" : album;
    item.durationMs = durationMs;
    item.format = format.isEmpty() ? (filePath.contains("://") ? "STREAM" : QFileInfo(filePath).suffix().toUpper()) : format;

    m_items.append(item);
    rebuildShuffleOrder();
    emit playlistUpdated();
}

void PlaylistManager::addFiles(const QStringList &filePaths)
{
    bool added = false;
    for (const QString &path : filePaths) {
        QFileInfo fi(path);
        if (fi.exists() && isSupportedMedia(path)) {
            UI::PlaylistItem item;
            item.filePath = path;
            item.title = fi.baseName();
            item.format = fi.suffix().toUpper();
            m_items.append(item);
            added = true;
        }
    }
    if (added) {
        rebuildShuffleOrder();
        emit playlistUpdated();
    }
}

void PlaylistManager::addDirectory(const QString &dirPath, bool recursive)
{
    if (dirPath.isEmpty()) return;
    QDir dir(dirPath);
    if (!dir.exists()) return;

    QStringList filters;
    for (const QString &ext : supportedAudioExtensions()) {
        filters.append("*." + ext);
    }
    for (const QString &ext : supportedVideoExtensions()) {
        filters.append("*." + ext);
    }

    QDirIterator::IteratorFlags flags = recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags;
    QDirIterator it(dirPath, filters, QDir::Files, flags);

    QStringList found;
    while (it.hasNext()) {
        found.append(it.next());
    }
    std::sort(found.begin(), found.end());

    addFiles(found);
}

void PlaylistManager::importMimeData(const QMimeData *mimeData)
{
    if (!mimeData || !mimeData->hasUrls()) return;

    QStringList filesToAdd;
    for (const QUrl &url : mimeData->urls()) {
        if (!url.isLocalFile()) continue;
        QString path = url.toLocalFile();
        QFileInfo fi(path);
        if (fi.isDir()) {
            addDirectory(path, true);
        } else if (fi.isFile() && isSupportedMedia(path)) {
            filesToAdd.append(path);
        }
    }

    if (!filesToAdd.isEmpty()) {
        addFiles(filesToAdd);
    }
}

void PlaylistManager::setItems(const QList<UI::PlaylistItem> &items, int currentIndex)
{
    m_items = items;
    m_currentIndex = (currentIndex >= 0 && currentIndex < m_items.size()) ? currentIndex : (m_items.isEmpty() ? -1 : 0);
    rebuildShuffleOrder();
    emit playlistUpdated();
    if (m_currentIndex >= 0 && m_currentIndex < m_items.size()) {
        emit currentTrackChanged(m_currentIndex, m_items[m_currentIndex]);
    }
}

void PlaylistManager::removeItem(int index)
{
    if (index < 0 || index >= m_items.size()) return;

    m_items.removeAt(index);
    if (m_items.isEmpty()) {
        m_currentIndex = -1;
    } else if (m_currentIndex >= m_items.size()) {
        m_currentIndex = m_items.size() - 1;
    } else if (index < m_currentIndex) {
        m_currentIndex--;
    }

    rebuildShuffleOrder();
    emit playlistUpdated();
    if (m_currentIndex >= 0 && m_currentIndex < m_items.size()) {
        emit currentTrackChanged(m_currentIndex, m_items[m_currentIndex]);
    }
}

void PlaylistManager::moveItem(int fromIndex, int toIndex)
{
    if (fromIndex < 0 || fromIndex >= m_items.size() || toIndex < 0 || toIndex >= m_items.size() || fromIndex == toIndex) {
        return;
    }

    UI::PlaylistItem it = m_items.takeAt(fromIndex);
    m_items.insert(toIndex, it);

    if (m_currentIndex == fromIndex) {
        m_currentIndex = toIndex;
    } else if (fromIndex < m_currentIndex && toIndex >= m_currentIndex) {
        m_currentIndex--;
    } else if (fromIndex > m_currentIndex && toIndex <= m_currentIndex) {
        m_currentIndex++;
    }

    rebuildShuffleOrder();
    emit playlistUpdated();
}

void PlaylistManager::clear()
{
    m_items.clear();
    m_shuffleOrder.clear();
    m_currentIndex = -1;
    emit playlistUpdated();
}

void PlaylistManager::setShuffle(bool enabled)
{
    if (m_shuffle != enabled) {
        m_shuffle = enabled;
        rebuildShuffleOrder();
        emit shuffleChanged(m_shuffle);
    }
}

void PlaylistManager::toggleShuffle()
{
    setShuffle(!m_shuffle);
}

void PlaylistManager::setLoopMode(LoopMode mode)
{
    if (m_loopMode != mode) {
        m_loopMode = mode;
        emit loopModeChanged(m_loopMode);
    }
}

void PlaylistManager::cycleLoopMode()
{
    int nextMode = (static_cast<int>(m_loopMode) + 1) % 3;
    setLoopMode(static_cast<LoopMode>(nextMode));
}

void PlaylistManager::rebuildShuffleOrder()
{
    m_shuffleOrder.clear();
    m_shuffleOrder.reserve(m_items.size());
    for (int i = 0; i < m_items.size(); ++i) {
        m_shuffleOrder.append(i);
    }

    if (m_shuffle && m_items.size() > 1) {
        if (m_currentIndex >= 0 && m_currentIndex < m_items.size()) {
            int curPos = m_shuffleOrder.indexOf(m_currentIndex);
            if (curPos != 0 && curPos >= 0) {
                std::swap(m_shuffleOrder[0], m_shuffleOrder[curPos]);
            }
            for (int i = m_shuffleOrder.size() - 1; i > 1; --i) {
                int j = 1 + QRandomGenerator::global()->bounded(i);
                std::swap(m_shuffleOrder[i], m_shuffleOrder[j]);
            }
        } else {
            // Full Fisher-Yates shuffle
            for (int i = m_shuffleOrder.size() - 1; i > 0; --i) {
                int j = QRandomGenerator::global()->bounded(i + 1);
                std::swap(m_shuffleOrder[i], m_shuffleOrder[j]);
            }
        }
    }
}

bool PlaylistManager::next()
{
    if (m_items.isEmpty()) return false;

    if (m_loopMode == LoopMode::Track) {
        // Repeat current track
        if (m_currentIndex >= 0 && m_currentIndex < m_items.size()) {
            emit currentTrackChanged(m_currentIndex, m_items[m_currentIndex]);
            return true;
        }
    }

    if (m_shuffle) {
        int posInShuffle = m_shuffleOrder.indexOf(m_currentIndex);
        if (posInShuffle >= 0 && posInShuffle + 1 < m_shuffleOrder.size()) {
            m_currentIndex = m_shuffleOrder[posInShuffle + 1];
            emit currentTrackChanged(m_currentIndex, m_items[m_currentIndex]);
            return true;
        } else if (m_loopMode == LoopMode::Playlist && !m_shuffleOrder.isEmpty()) {
            rebuildShuffleOrder();
            m_currentIndex = m_shuffleOrder.first();
            emit currentTrackChanged(m_currentIndex, m_items[m_currentIndex]);
            return true;
        }
    } else {
        if (m_currentIndex + 1 < m_items.size()) {
            m_currentIndex++;
            emit currentTrackChanged(m_currentIndex, m_items[m_currentIndex]);
            return true;
        } else if (m_loopMode == LoopMode::Playlist) {
            m_currentIndex = 0;
            emit currentTrackChanged(m_currentIndex, m_items[m_currentIndex]);
            return true;
        }
    }

    emit queueEnded();
    return false;
}

bool PlaylistManager::previous()
{
    if (m_items.isEmpty()) return false;

    if (m_loopMode == LoopMode::Track) {
        if (m_currentIndex >= 0 && m_currentIndex < m_items.size()) {
            emit currentTrackChanged(m_currentIndex, m_items[m_currentIndex]);
            return true;
        }
    }

    if (m_shuffle) {
        int posInShuffle = m_shuffleOrder.indexOf(m_currentIndex);
        if (posInShuffle > 0) {
            m_currentIndex = m_shuffleOrder[posInShuffle - 1];
            emit currentTrackChanged(m_currentIndex, m_items[m_currentIndex]);
            return true;
        } else if (m_loopMode == LoopMode::Playlist && !m_shuffleOrder.isEmpty()) {
            m_currentIndex = m_shuffleOrder.last();
            emit currentTrackChanged(m_currentIndex, m_items[m_currentIndex]);
            return true;
        }
    } else {
        if (m_currentIndex > 0) {
            m_currentIndex--;
            emit currentTrackChanged(m_currentIndex, m_items[m_currentIndex]);
            return true;
        } else if (m_loopMode == LoopMode::Playlist) {
            m_currentIndex = m_items.size() - 1;
            emit currentTrackChanged(m_currentIndex, m_items[m_currentIndex]);
            return true;
        }
    }

    return false;
}

} // namespace Library
} // namespace Penguin
