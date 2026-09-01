#ifndef STATEPERSISTENCE_H
#define STATEPERSISTENCE_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QVector>
#include <QList>

#include "DatabaseManager.h"
#include "PlaylistMatrixWidget.h"

namespace Penguin {
namespace Library {

class StatePersistence : public QObject {
    Q_OBJECT

public:
    explicit StatePersistence(DatabaseManager *dbManager, QObject *parent = nullptr);
    virtual ~StatePersistence() = default;

    DatabaseManager* databaseManager() const { return m_db; }

    // Window Geometry & State
    void saveWindowState(const QWidget *window);
    void restoreWindowState(QWidget *window);

    // Audio & Equalizer Settings
    void saveAudioSettings(int volume, bool isMuted, double speed, const QString &eqPreset, const QVector<double> &eqGains);
    void restoreAudioSettings(int &volume, bool &isMuted, double &speed, QString &eqPreset, QVector<double> &eqGains);

    // UI Mode & Diagnostics
    void saveUIMode(int mode);
    int restoreUIMode(int defaultMode = 0);
    void saveTelemetryPreferences(bool osdEnabled, bool reticlesEnabled);
    void restoreTelemetryPreferences(bool &osdEnabled, bool &reticlesEnabled);

    // Playlist Queue
    void saveCurrentPlaylist(const QList<UI::PlaylistItem> &items, int currentIndex);
    QList<UI::PlaylistItem> restoreCurrentPlaylist(int &currentIndex);

    // Playback Resume Position
    void recordPlayback(const QString &uri, const QString &title, const QString &artist, const QString &album,
                        qint64 durationMs, qint64 positionMs, const QString &mediaType = "audio");
    qint64 getResumePosition(const QString &uri) const;

private:
    DatabaseManager *m_db = nullptr;
};

} // namespace Library
} // namespace Penguin

#endif // STATEPERSISTENCE_H
