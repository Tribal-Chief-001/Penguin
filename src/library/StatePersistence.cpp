#include "StatePersistence.h"

#include <QScreen>
#include <QGuiApplication>
#include <QVariantMap>
#include <QVariantList>

namespace Penguin {
namespace Library {

StatePersistence::StatePersistence(DatabaseManager *dbManager, QObject *parent)
    : QObject(parent)
    , m_db(dbManager)
{
}

void StatePersistence::saveWindowState(const QWidget *window)
{
    if (!m_db || !window) return;

    QVariantMap map;
    map["x"] = window->x();
    map["y"] = window->y();
    map["width"] = window->width();
    map["height"] = window->height();
    map["is_maximized"] = window->isMaximized();
    map["is_fullscreen"] = window->isFullScreen();

    m_db->setSetting("window_geometry", map);
}

void StatePersistence::restoreWindowState(QWidget *window)
{
    if (!m_db || !window) return;

    QVariant val = m_db->getSetting("window_geometry");
    if (!val.isValid()) return;

    QVariantMap map = val.toMap();
    if (map.isEmpty()) return;

    int x = map.value("x", 100).toInt();
    int y = map.value("y", 100).toInt();
    int w = map.value("width", 1280).toInt();
    int h = map.value("height", 720).toInt();
    bool isMaximized = map.value("is_maximized", false).toBool();
    bool isFullscreen = map.value("is_fullscreen", false).toBool();

    // Multi-monitor sanity check
    bool onScreen = false;
    const auto screens = QGuiApplication::screens();
    for (const auto *screen : screens) {
        if (screen->geometry().intersects(QRect(x, y, w, h))) {
            onScreen = true;
            break;
        }
    }

    if (onScreen) {
        window->setGeometry(x, y, w, h);
    } else {
        window->resize(w, h);
    }

    if (isFullscreen) {
        window->showFullScreen();
    } else if (isMaximized) {
        window->showMaximized();
    }
}

void StatePersistence::saveAudioSettings(int volume, bool isMuted, double speed, const QString &eqPreset, const QVector<double> &eqGains)
{
    if (!m_db) return;

    QVariantMap map;
    map["volume"] = volume;
    map["is_muted"] = isMuted;
    map["speed"] = speed;
    map["eq_preset"] = eqPreset;

    QVariantList gainsList;
    for (double g : eqGains) {
        gainsList.append(g);
    }
    map["eq_gains"] = gainsList;

    m_db->setSetting("audio_settings", map);
}

void StatePersistence::restoreAudioSettings(int &volume, bool &isMuted, double &speed, QString &eqPreset, QVector<double> &eqGains)
{
    if (!m_db) return;

    QVariant val = m_db->getSetting("audio_settings");
    if (!val.isValid()) return;

    QVariantMap map = val.toMap();
    if (map.contains("volume")) {
        volume = map["volume"].toInt();
    }
    if (map.contains("is_muted")) {
        isMuted = map["is_muted"].toBool();
    }
    if (map.contains("speed")) {
        speed = map["speed"].toDouble();
    }
    if (map.contains("eq_preset")) {
        eqPreset = map["eq_preset"].toString();
    }
    if (map.contains("eq_gains")) {
        QVariantList gainsList = map["eq_gains"].toList();
        if (gainsList.size() == 10) {
            eqGains.resize(10);
            for (int i = 0; i < 10; ++i) {
                eqGains[i] = gainsList[i].toDouble();
            }
        }
    }
}

void StatePersistence::saveUIMode(int mode)
{
    if (!m_db) return;
    m_db->setSetting("ui_mode", mode);
}

int StatePersistence::restoreUIMode(int defaultMode)
{
    if (!m_db) return defaultMode;
    QVariant val = m_db->getSetting("ui_mode", defaultMode);
    return val.toInt();
}

void StatePersistence::saveTelemetryPreferences(bool osdEnabled, bool reticlesEnabled)
{
    if (!m_db) return;
    QVariantMap map;
    map["osd_enabled"] = osdEnabled;
    map["reticles_enabled"] = reticlesEnabled;
    m_db->setSetting("telemetry_preferences", map);
}

void StatePersistence::restoreTelemetryPreferences(bool &osdEnabled, bool &reticlesEnabled)
{
    if (!m_db) return;
    QVariant val = m_db->getSetting("telemetry_preferences");
    if (!val.isValid()) return;

    QVariantMap map = val.toMap();
    if (map.contains("osd_enabled")) {
        osdEnabled = map["osd_enabled"].toBool();
    }
    if (map.contains("reticles_enabled")) {
        reticlesEnabled = map["reticles_enabled"].toBool();
    }
}

void StatePersistence::saveCurrentPlaylist(const QList<UI::PlaylistItem> &items, int currentIndex)
{
    if (!m_db) return;

    QList<PlaylistItemRecord> records;
    records.reserve(items.size());
    for (int i = 0; i < items.size(); ++i) {
        PlaylistItemRecord rec;
        rec.playlistName = "default";
        rec.positionOrder = i;
        rec.uri = items[i].filePath;
        rec.title = items[i].title;
        rec.artist = items[i].artist;
        rec.album = items[i].album;
        rec.durationMs = items[i].durationMs;
        rec.format = items[i].format;
        records.append(rec);
    }

    m_db->savePlaylistItems("default", records);
    m_db->setSetting("current_playlist_index", currentIndex);
}

QList<UI::PlaylistItem> StatePersistence::restoreCurrentPlaylist(int &currentIndex)
{
    QList<UI::PlaylistItem> result;
    if (!m_db) return result;

    QList<PlaylistItemRecord> records = m_db->getPlaylistItems("default");
    result.reserve(records.size());
    for (const auto &rec : records) {
        UI::PlaylistItem item;
        item.filePath = rec.uri;
        item.title = rec.title;
        item.artist = rec.artist;
        item.album = rec.album;
        item.durationMs = rec.durationMs;
        item.format = rec.format;
        result.append(item);
    }

    currentIndex = m_db->getSetting("current_playlist_index", -1).toInt();
    return result;
}

void StatePersistence::recordPlayback(const QString &uri, const QString &title, const QString &artist, const QString &album,
                                      qint64 durationMs, qint64 positionMs, const QString &mediaType)
{
    if (!m_db || uri.isEmpty()) return;
    m_db->recordPlayback(uri, title, artist, album, durationMs, positionMs, mediaType);
}

qint64 StatePersistence::getResumePosition(const QString &uri) const
{
    if (!m_db || uri.isEmpty()) return 0;
    HistoryItem item = m_db->getHistoryItem(uri);
    return item.lastPositionMs;
}

} // namespace Library
} // namespace Penguin
