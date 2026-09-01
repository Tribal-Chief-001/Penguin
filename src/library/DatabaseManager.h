#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVector>
#include <QVariant>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

namespace Penguin {
namespace Library {

struct HistoryItem {
    int id = 0;
    QString uri;
    QString title;
    QString artist;
    QString album;
    qint64 durationMs = 0;
    qint64 lastPositionMs = 0;
    int playCount = 1;
    QDateTime lastPlayedAt;
    QString mediaType; // "audio", "video", "stream"
    qint64 fileSize = 0;
    QString coverArtPath;
};

struct PlaylistItemRecord {
    int id = 0;
    QString playlistName = "default";
    int positionOrder = 0;
    QString uri;
    QString title;
    QString artist;
    QString album;
    qint64 durationMs = 0;
    QString format;
};

struct EqualizerPresetRecord {
    int id = 0;
    QString name;
    QVector<double> gains; // 10 band gains in dB
    double preamp = 0.0;
    bool isBuiltin = false;
};

class DatabaseManager : public QObject {
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    virtual ~DatabaseManager();

    // Lifecycle
    bool initialize(const QString &customDbPath = QString());
    void close();
    bool isOpen() const;
    QString databasePath() const { return m_dbPath; }

    // Settings CRUD
    bool setSetting(const QString &key, const QVariant &value);
    QVariant getSetting(const QString &key, const QVariant &defaultValue = QVariant()) const;
    bool removeSetting(const QString &key);

    // Media Playback History CRUD
    bool recordPlayback(const QString &uri,
                        const QString &title = QString(),
                        const QString &artist = QString(),
                        const QString &album = QString(),
                        qint64 durationMs = 0,
                        qint64 lastPositionMs = 0,
                        const QString &mediaType = "audio",
                        qint64 fileSize = 0,
                        const QString &coverArtPath = QString());
    QList<HistoryItem> getRecentHistory(int limit = 50) const;
    HistoryItem getHistoryItem(const QString &uri) const;
    bool removeHistoryItem(const QString &uri);
    bool clearHistory();

    // Playlist Items CRUD
    bool savePlaylistItems(const QString &playlistName, const QList<PlaylistItemRecord> &items);
    QList<PlaylistItemRecord> getPlaylistItems(const QString &playlistName = "default") const;
    bool clearPlaylist(const QString &playlistName = "default");
    QStringList getSavedPlaylistNames() const;

    // Equalizer Presets CRUD
    bool saveEqualizerPreset(const QString &name, const QVector<double> &gains, double preamp = 0.0, bool isBuiltin = false);
    EqualizerPresetRecord getEqualizerPreset(const QString &name) const;
    QList<EqualizerPresetRecord> getAllEqualizerPresets() const;
    bool deleteEqualizerPreset(const QString &name);

private:
    bool createTables();
    bool seedFactoryPresets();

    QString m_connectionName;
    QString m_dbPath;
    bool m_initialized = false;
};

} // namespace Library
} // namespace Penguin

#endif // DATABASEMANAGER_H
