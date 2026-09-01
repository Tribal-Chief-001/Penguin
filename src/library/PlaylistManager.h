#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMimeData>

#include "PlaylistMatrixWidget.h"

namespace Penguin {
namespace Library {

enum class LoopMode {
    None = 0,
    Track = 1,
    Playlist = 2
};

class PlaylistManager : public QObject {
    Q_OBJECT

public:
    explicit PlaylistManager(QObject *parent = nullptr);
    virtual ~PlaylistManager() = default;

    int count() const { return m_items.size(); }
    int currentIndex() const { return m_currentIndex; }
    const QList<UI::PlaylistItem>& items() const { return m_items; }
    UI::PlaylistItem itemAt(int index) const;
    UI::PlaylistItem currentItem() const;

    bool isShuffleEnabled() const { return m_shuffle; }
    LoopMode loopMode() const { return m_loopMode; }

    bool hasNext() const;
    bool hasPrevious() const;

    // File format filters
    static QStringList supportedAudioExtensions();
    static QStringList supportedVideoExtensions();
    static QStringList supportedSubtitleExtensions();
    static bool isSupportedMedia(const QString &filePath);

public slots:
    void setCurrentIndex(int index);
    void addFile(const QString &filePath);
    void addFiles(const QStringList &filePaths);
    void addDirectory(const QString &dirPath, bool recursive = true);
    void importMimeData(const QMimeData *mimeData);
    void setItems(const QList<UI::PlaylistItem> &items, int currentIndex = -1);

    void removeItem(int index);
    void moveItem(int fromIndex, int toIndex);
    void clear();

    void setShuffle(bool enabled);
    void toggleShuffle();
    void setLoopMode(LoopMode mode);
    void cycleLoopMode();

    bool next();
    bool previous();

signals:
    void currentTrackChanged(int index, const UI::PlaylistItem &item);
    void playlistUpdated();
    void shuffleChanged(bool enabled);
    void loopModeChanged(LoopMode mode);
    void queueEnded();

private:
    void rebuildShuffleOrder();

    QList<UI::PlaylistItem> m_items;
    QList<int> m_shuffleOrder;
    int m_currentIndex = -1;
    bool m_shuffle = false;
    LoopMode m_loopMode = LoopMode::None;
};

} // namespace Library
} // namespace Penguin

#endif // PLAYLISTMANAGER_H
