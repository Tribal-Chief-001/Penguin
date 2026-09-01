#include "DatabaseManager.h"

#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>
#include <unistd.h>

namespace Penguin {
namespace Library {

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
    , m_connectionName(QString("penguin_db_conn_%1").arg(reinterpret_cast<quintptr>(this)))
{
}

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::initialize(const QString &customDbPath)
{
    if (m_initialized) {
        return true;
    }

    if (customDbPath.isEmpty()) {
        QString dataDir;
        QString xdgData = qEnvironmentVariable("XDG_DATA_HOME");
        if (!xdgData.isEmpty()) {
            dataDir = xdgData + "/penguin";
        } else {
            QString genericData = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
            if (!genericData.isEmpty() && QDir().mkpath(genericData + "/penguin")) {
                dataDir = genericData + "/penguin";
            }
        }

        if (dataDir.isEmpty() || !QDir().mkpath(dataDir)) {
            dataDir = QDir::tempPath() + QString("/penguin_%1").arg(getuid());
            QDir().mkpath(dataDir);
        }

        m_dbPath = dataDir + "/penguin.db";
    } else {
        m_dbPath = customDbPath;
        QFileInfo fi(m_dbPath);
        QDir().mkpath(fi.absolutePath());
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    db.setDatabaseName(m_dbPath);

    if (!db.open()) {
        qWarning() << "DatabaseManager: Failed to open SQLite database at" << m_dbPath << ":" << db.lastError().text();
        return false;
    }

    // Configure SQLite PRAGMAs for high-performance WAL mode & reliability
    QSqlQuery pragmaQuery(db);
    pragmaQuery.exec("PRAGMA journal_mode = WAL;");
    pragmaQuery.exec("PRAGMA synchronous = NORMAL;");
    pragmaQuery.exec("PRAGMA foreign_keys = ON;");
    pragmaQuery.exec("PRAGMA busy_timeout = 5000;");

    if (!createTables()) {
        qWarning() << "DatabaseManager: Failed to create database schema.";
        db.close();
        return false;
    }

    m_initialized = true;
    seedFactoryPresets();

    return true;
}

void DatabaseManager::close()
{
    if (m_initialized) {
        {
            QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
            if (db.isOpen()) {
                db.close();
            }
        }
        QSqlDatabase::removeDatabase(m_connectionName);
        m_initialized = false;
    }
}

bool DatabaseManager::isOpen() const
{
    if (!m_initialized) return false;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    return db.isOpen();
}

bool DatabaseManager::createTables()
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    if (!db.isOpen()) return false;

    QSqlQuery q(db);

    // Schema version
    if (!q.exec("CREATE TABLE IF NOT EXISTS schema_version ("
                "version INTEGER PRIMARY KEY, "
                "applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);")) {
        qWarning() << "DB schema_version create error:" << q.lastError().text();
        return false;
    }

    // Key-Value App Settings
    if (!q.exec("CREATE TABLE IF NOT EXISTS app_settings ("
                "key TEXT PRIMARY KEY, "
                "value TEXT NOT NULL, "
                "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);")) {
        qWarning() << "DB app_settings create error:" << q.lastError().text();
        return false;
    }

    // Media Playback History
    if (!q.exec("CREATE TABLE IF NOT EXISTS media_history ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "uri TEXT UNIQUE NOT NULL, "
                "title TEXT, "
                "artist TEXT, "
                "album TEXT, "
                "duration_ms INTEGER DEFAULT 0, "
                "last_position_ms INTEGER DEFAULT 0, "
                "play_count INTEGER DEFAULT 1, "
                "last_played_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                "media_type TEXT CHECK(media_type IN ('audio', 'video', 'stream')) NOT NULL DEFAULT 'audio', "
                "file_size INTEGER DEFAULT 0, "
                "cover_art_path TEXT);")) {
        qWarning() << "DB media_history create error:" << q.lastError().text();
        return false;
    }

    // Saved Playlists Master
    if (!q.exec("CREATE TABLE IF NOT EXISTS saved_playlists ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "name TEXT UNIQUE NOT NULL, "
                "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);")) {
        qWarning() << "DB saved_playlists create error:" << q.lastError().text();
        return false;
    }

    // Playlist Items Matrix
    if (!q.exec("CREATE TABLE IF NOT EXISTS playlist_items ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "playlist_name TEXT NOT NULL DEFAULT 'default', "
                "position_order INTEGER NOT NULL, "
                "uri TEXT NOT NULL, "
                "title TEXT, "
                "artist TEXT, "
                "album TEXT, "
                "duration_ms INTEGER DEFAULT 0, "
                "format TEXT, "
                "added_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                "UNIQUE(playlist_name, position_order));")) {
        qWarning() << "DB playlist_items create error:" << q.lastError().text();
        return false;
    }

    // 10-Band Graphic Equalizer Presets
    if (!q.exec("CREATE TABLE IF NOT EXISTS equalizer_presets ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "name TEXT UNIQUE NOT NULL, "
                "band_32hz REAL DEFAULT 0.0, "
                "band_64hz REAL DEFAULT 0.0, "
                "band_125hz REAL DEFAULT 0.0, "
                "band_250hz REAL DEFAULT 0.0, "
                "band_500hz REAL DEFAULT 0.0, "
                "band_1khz REAL DEFAULT 0.0, "
                "band_2khz REAL DEFAULT 0.0, "
                "band_4khz REAL DEFAULT 0.0, "
                "band_8khz REAL DEFAULT 0.0, "
                "band_16khz REAL DEFAULT 0.0, "
                "preamp REAL DEFAULT 0.0, "
                "is_builtin BOOLEAN DEFAULT 0);")) {
        qWarning() << "DB equalizer_presets create error:" << q.lastError().text();
        return false;
    }

    // Indexes
    q.exec("CREATE INDEX IF NOT EXISTS idx_history_last_played ON media_history(last_played_at DESC);");
    q.exec("CREATE INDEX IF NOT EXISTS idx_playlist_order ON playlist_items(playlist_name, position_order ASC);");

    // Insert version 1
    q.exec("INSERT OR IGNORE INTO schema_version (version) VALUES (1);");

    return true;
}

bool DatabaseManager::seedFactoryPresets()
{
    struct PresetDef {
        const char *name;
        double gains[10];
        double preamp;
    };

    static const PresetDef factoryPresets[] = {
        {"Flat", {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, 0.0},
        {"Bass Boost", {+6.0, +5.0, +3.5, +1.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, -2.0},
        {"Studio Mastering", {+1.0, +1.5, 0.0, -0.5, +0.5, +1.0, +1.5, +2.0, +2.5, +2.0}, -1.0},
        {"Vocal Clarity", {-2.0, -1.0, 0.0, +1.5, +3.0, +3.5, +3.0, +1.5, 0.0, -1.0}, 0.0},
        {"Electronic / Synth", {+5.5, +4.5, +2.0, 0.0, -1.0, +1.5, +2.5, +3.5, +4.5, +4.0}, -2.0},
        {"Acoustic / Live", {+2.0, +1.5, +1.0, 0.0, +1.0, +2.0, +2.5, +3.0, +3.5, +3.0}, -1.0},
        {"Rock / Metal", {+4.5, +3.5, +1.0, -0.5, -1.5, +1.0, +2.0, +3.5, +4.0, +4.5}, -1.5},
        {"Night Mode (Low Dynamic)", {-4.0, -3.0, -1.5, 0.0, +1.0, +2.0, +2.0, +1.0, -1.0, -3.0}, +1.0}
    };

    for (const auto &p : factoryPresets) {
        QVector<double> gains(10);
        for (int i = 0; i < 10; ++i) {
            gains[i] = p.gains[i];
        }
        // Save if not present
        EqualizerPresetRecord existing = getEqualizerPreset(QString::fromUtf8(p.name));
        if (existing.name.isEmpty()) {
            saveEqualizerPreset(QString::fromUtf8(p.name), gains, p.preamp, true);
        }
    }
    return true;
}

bool DatabaseManager::setSetting(const QString &key, const QVariant &value)
{
    if (!isOpen()) return false;

    QJsonDocument doc;
    if (value.typeId() == QMetaType::QVariantMap) {
        doc = QJsonDocument::fromVariant(value.toMap());
    } else if (value.typeId() == QMetaType::QVariantList || value.typeId() == QMetaType::QStringList) {
        doc = QJsonDocument::fromVariant(value.toList());
    } else {
        QJsonObject obj;
        obj["val"] = QJsonValue::fromVariant(value);
        doc = QJsonDocument(obj);
    }
    QString jsonStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    QSqlQuery q(db);
    q.prepare("INSERT INTO app_settings (key, value, updated_at) "
              "VALUES (:key, :value, CURRENT_TIMESTAMP) "
              "ON CONFLICT(key) DO UPDATE SET value = excluded.value, updated_at = CURRENT_TIMESTAMP;");
    q.bindValue(":key", key);
    q.bindValue(":value", jsonStr);

    if (!q.exec()) {
        qWarning() << "DatabaseManager::setSetting failed:" << q.lastError().text();
        return false;
    }
    return true;
}

QVariant DatabaseManager::getSetting(const QString &key, const QVariant &defaultValue) const
{
    if (!isOpen()) return defaultValue;

    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    QSqlQuery q(db);
    q.prepare("SELECT value FROM app_settings WHERE key = :key;");
    q.bindValue(":key", key);

    if (!q.exec() || !q.next()) {
        return defaultValue;
    }

    QString jsonStr = q.value(0).toString();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        return jsonStr;
    }

    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("val") && obj.size() == 1) {
            return obj["val"].toVariant();
        }
        return obj.toVariantMap();
    } else if (doc.isArray()) {
        return doc.array().toVariantList();
    }
    return defaultValue;
}

bool DatabaseManager::removeSetting(const QString &key)
{
    if (!isOpen()) return false;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    QSqlQuery q(db);
    q.prepare("DELETE FROM app_settings WHERE key = :key;");
    q.bindValue(":key", key);
    return q.exec();
}

bool DatabaseManager::recordPlayback(const QString &uri,
                                     const QString &title,
                                     const QString &artist,
                                     const QString &album,
                                     qint64 durationMs,
                                     qint64 lastPositionMs,
                                     const QString &mediaType,
                                     qint64 fileSize,
                                     const QString &coverArtPath)
{
    if (!isOpen() || uri.isEmpty()) return false;

    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    QSqlQuery q(db);
    q.prepare("INSERT INTO media_history "
              "(uri, title, artist, album, duration_ms, last_position_ms, media_type, file_size, cover_art_path, play_count, last_played_at) "
              "VALUES (:uri, :title, :artist, :album, :duration_ms, :last_position_ms, :media_type, :file_size, :cover_art_path, 1, CURRENT_TIMESTAMP) "
              "ON CONFLICT(uri) DO UPDATE SET "
              "title = excluded.title, "
              "artist = excluded.artist, "
              "album = excluded.album, "
              "duration_ms = excluded.duration_ms, "
              "last_position_ms = excluded.last_position_ms, "
              "media_type = excluded.media_type, "
              "file_size = excluded.file_size, "
              "cover_art_path = excluded.cover_art_path, "
              "play_count = media_history.play_count + 1, "
              "last_played_at = CURRENT_TIMESTAMP;");

    q.bindValue(":uri", uri);
    q.bindValue(":title", title);
    q.bindValue(":artist", artist);
    q.bindValue(":album", album);
    q.bindValue(":duration_ms", durationMs);
    q.bindValue(":last_position_ms", lastPositionMs);
    q.bindValue(":media_type", mediaType);
    q.bindValue(":file_size", fileSize);
    q.bindValue(":cover_art_path", coverArtPath);

    if (!q.exec()) {
        qWarning() << "DatabaseManager::recordPlayback failed:" << q.lastError().text();
        return false;
    }
    return true;
}

QList<HistoryItem> DatabaseManager::getRecentHistory(int limit) const
{
    QList<HistoryItem> list;
    if (!isOpen()) return list;

    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    QSqlQuery q(db);
    q.prepare("SELECT id, uri, title, artist, album, duration_ms, last_position_ms, play_count, last_played_at, media_type, file_size, cover_art_path "
              "FROM media_history ORDER BY last_played_at DESC LIMIT :limit;");
    q.bindValue(":limit", limit);

    if (q.exec()) {
        while (q.next()) {
            HistoryItem it;
            it.id = q.value(0).toInt();
            it.uri = q.value(1).toString();
            it.title = q.value(2).toString();
            it.artist = q.value(3).toString();
            it.album = q.value(4).toString();
            it.durationMs = q.value(5).toLongLong();
            it.lastPositionMs = q.value(6).toLongLong();
            it.playCount = q.value(7).toInt();
            it.lastPlayedAt = q.value(8).toDateTime();
            it.mediaType = q.value(9).toString();
            it.fileSize = q.value(10).toLongLong();
            it.coverArtPath = q.value(11).toString();
            list.append(it);
        }
    }
    return list;
}

HistoryItem DatabaseManager::getHistoryItem(const QString &uri) const
{
    HistoryItem it;
    if (!isOpen() || uri.isEmpty()) return it;

    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    QSqlQuery q(db);
    q.prepare("SELECT id, uri, title, artist, album, duration_ms, last_position_ms, play_count, last_played_at, media_type, file_size, cover_art_path "
              "FROM media_history WHERE uri = :uri;");
    q.bindValue(":uri", uri);

    if (q.exec() && q.next()) {
        it.id = q.value(0).toInt();
        it.uri = q.value(1).toString();
        it.title = q.value(2).toString();
        it.artist = q.value(3).toString();
        it.album = q.value(4).toString();
        it.durationMs = q.value(5).toLongLong();
        it.lastPositionMs = q.value(6).toLongLong();
        it.playCount = q.value(7).toInt();
        it.lastPlayedAt = q.value(8).toDateTime();
        it.mediaType = q.value(9).toString();
        it.fileSize = q.value(10).toLongLong();
        it.coverArtPath = q.value(11).toString();
    }
    return it;
}

bool DatabaseManager::removeHistoryItem(const QString &uri)
{
    if (!isOpen() || uri.isEmpty()) return false;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    QSqlQuery q(db);
    q.prepare("DELETE FROM media_history WHERE uri = :uri;");
    q.bindValue(":uri", uri);
    return q.exec();
}

bool DatabaseManager::clearHistory()
{
    if (!isOpen()) return false;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    QSqlQuery q(db);
    return q.exec("DELETE FROM media_history;");
}

bool DatabaseManager::savePlaylistItems(const QString &playlistName, const QList<PlaylistItemRecord> &items)
{
    if (!isOpen()) return false;

    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    if (!db.transaction()) return false;

    QSqlQuery qDelete(db);
    qDelete.prepare("DELETE FROM playlist_items WHERE playlist_name = :pname;");
    qDelete.bindValue(":pname", playlistName);
    if (!qDelete.exec()) {
        db.rollback();
        return false;
    }

    QSqlQuery qInsert(db);
    qInsert.prepare("INSERT INTO playlist_items "
                    "(playlist_name, position_order, uri, title, artist, album, duration_ms, format) "
                    "VALUES (:pname, :pos, :uri, :title, :artist, :album, :dur, :fmt);");

    for (int i = 0; i < items.size(); ++i) {
        const auto &it = items[i];
        qInsert.bindValue(":pname", playlistName);
        qInsert.bindValue(":pos", i);
        qInsert.bindValue(":uri", it.uri);
        qInsert.bindValue(":title", it.title);
        qInsert.bindValue(":artist", it.artist);
        qInsert.bindValue(":album", it.album);
        qInsert.bindValue(":dur", it.durationMs);
        qInsert.bindValue(":fmt", it.format);

        if (!qInsert.exec()) {
            qWarning() << "DatabaseManager::savePlaylistItems insert error at" << i << ":" << qInsert.lastError().text();
            db.rollback();
            return false;
        }
    }

    // Also upsert saved_playlists
    QSqlQuery qPlaylist(db);
    qPlaylist.prepare("INSERT INTO saved_playlists (name, updated_at) "
                      "VALUES (:name, CURRENT_TIMESTAMP) "
                      "ON CONFLICT(name) DO UPDATE SET updated_at = CURRENT_TIMESTAMP;");
    qPlaylist.bindValue(":name", playlistName);
    qPlaylist.exec();

    return db.commit();
}

QList<PlaylistItemRecord> DatabaseManager::getPlaylistItems(const QString &playlistName) const
{
    QList<PlaylistItemRecord> list;
    if (!isOpen()) return list;

    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    QSqlQuery q(db);
    q.prepare("SELECT id, playlist_name, position_order, uri, title, artist, album, duration_ms, format "
              "FROM playlist_items WHERE playlist_name = :pname ORDER BY position_order ASC;");
    q.bindValue(":pname", playlistName);

    if (q.exec()) {
        while (q.next()) {
            PlaylistItemRecord it;
            it.id = q.value(0).toInt();
            it.playlistName = q.value(1).toString();
            it.positionOrder = q.value(2).toInt();
            it.uri = q.value(3).toString();
            it.title = q.value(4).toString();
            it.artist = q.value(5).toString();
            it.album = q.value(6).toString();
            it.durationMs = q.value(7).toLongLong();
            it.format = q.value(8).toString();
            list.append(it);
        }
    }
    return list;
}

bool DatabaseManager::clearPlaylist(const QString &playlistName)
{
    if (!isOpen()) return false;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    QSqlQuery q(db);
    q.prepare("DELETE FROM playlist_items WHERE playlist_name = :pname;");
    q.bindValue(":pname", playlistName);
    return q.exec();
}

QStringList DatabaseManager::getSavedPlaylistNames() const
{
    QStringList names;
    if (!isOpen()) return names;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    QSqlQuery q(db);
    if (q.exec("SELECT name FROM saved_playlists ORDER BY name ASC;")) {
        while (q.next()) {
            names.append(q.value(0).toString());
        }
    }
    return names;
}

bool DatabaseManager::saveEqualizerPreset(const QString &name, const QVector<double> &gains, double preamp, bool isBuiltin)
{
    if (!isOpen() || name.isEmpty() || gains.size() != 10) return false;

    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    QSqlQuery q(db);
    q.prepare("INSERT INTO equalizer_presets "
              "(name, band_32hz, band_64hz, band_125hz, band_250hz, band_500hz, band_1khz, band_2khz, band_4khz, band_8khz, band_16khz, preamp, is_builtin) "
              "VALUES (:name, :b0, :b1, :b2, :b3, :b4, :b5, :b6, :b7, :b8, :b9, :preamp, :builtin) "
              "ON CONFLICT(name) DO UPDATE SET "
              "band_32hz = excluded.band_32hz, "
              "band_64hz = excluded.band_64hz, "
              "band_125hz = excluded.band_125hz, "
              "band_250hz = excluded.band_250hz, "
              "band_500hz = excluded.band_500hz, "
              "band_1khz = excluded.band_1khz, "
              "band_2khz = excluded.band_2khz, "
              "band_4khz = excluded.band_4khz, "
              "band_8khz = excluded.band_8khz, "
              "band_16khz = excluded.band_16khz, "
              "preamp = excluded.preamp, "
              "is_builtin = excluded.is_builtin;");

    q.bindValue(":name", name);
    for (int i = 0; i < 10; ++i) {
        q.bindValue(QString(":b%1").arg(i), gains[i]);
    }
    q.bindValue(":preamp", preamp);
    q.bindValue(":builtin", isBuiltin ? 1 : 0);

    if (!q.exec()) {
        qWarning() << "DatabaseManager::saveEqualizerPreset error:" << q.lastError().text();
        return false;
    }
    return true;
}

EqualizerPresetRecord DatabaseManager::getEqualizerPreset(const QString &name) const
{
    EqualizerPresetRecord rec;
    if (!isOpen() || name.isEmpty()) return rec;

    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    QSqlQuery q(db);
    q.prepare("SELECT id, name, band_32hz, band_64hz, band_125hz, band_250hz, band_500hz, band_1khz, band_2khz, band_4khz, band_8khz, band_16khz, preamp, is_builtin "
              "FROM equalizer_presets WHERE name = :name;");
    q.bindValue(":name", name);

    if (q.exec() && q.next()) {
        rec.id = q.value(0).toInt();
        rec.name = q.value(1).toString();
        rec.gains.resize(10);
        for (int i = 0; i < 10; ++i) {
            rec.gains[i] = q.value(2 + i).toDouble();
        }
        rec.preamp = q.value(12).toDouble();
        rec.isBuiltin = q.value(13).toBool();
    }
    return rec;
}

QList<EqualizerPresetRecord> DatabaseManager::getAllEqualizerPresets() const
{
    QList<EqualizerPresetRecord> list;
    if (!isOpen()) return list;

    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    QSqlQuery q(db);
    if (q.exec("SELECT id, name, band_32hz, band_64hz, band_125hz, band_250hz, band_500hz, band_1khz, band_2khz, band_4khz, band_8khz, band_16khz, preamp, is_builtin "
               "FROM equalizer_presets ORDER BY is_builtin DESC, name ASC;")) {
        while (q.next()) {
            EqualizerPresetRecord rec;
            rec.id = q.value(0).toInt();
            rec.name = q.value(1).toString();
            rec.gains.resize(10);
            for (int i = 0; i < 10; ++i) {
                rec.gains[i] = q.value(2 + i).toDouble();
            }
            rec.preamp = q.value(12).toDouble();
            rec.isBuiltin = q.value(13).toBool();
            list.append(rec);
        }
    }
    return list;
}

bool DatabaseManager::deleteEqualizerPreset(const QString &name)
{
    if (!isOpen() || name.isEmpty()) return false;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
    QSqlQuery q(db);
    q.prepare("DELETE FROM equalizer_presets WHERE name = :name AND is_builtin = 0;");
    q.bindValue(":name", name);
    return q.exec();
}

} // namespace Library
} // namespace Penguin
