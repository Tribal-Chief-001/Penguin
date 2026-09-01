#include "MainWindow.h"
#include "BrutalistTheme.h"
#include "StatePersistence.h"
#include "PlaylistManager.h"

#include <QKeyEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileDialog>
#include <QDirIterator>
#include <QFileInfo>
#include <QMessageBox>
#include <QShortcut>

namespace Penguin {
namespace UI {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_engine(std::make_unique<Core::PlaybackEngine>(this))
{
    // Initialize engine in GUI (onscreen video enabled) mode
    m_engine->initialize(false);

    setupUI();
    setupConnections();
    setupShortcuts();

    setFocusPolicy(Qt::StrongFocus);
    setAcceptDrops(true);
    resize(1280, 720);
    setWindowTitle("Penguin // Tactile Digital Brutalist Media Player");
    setStyleSheet(BrutalistTheme::globalStyleSheet());
}

void MainWindow::setupUI()
{
    m_stackedWidget = new QStackedWidget(this);

    // Index 0: Video Viewfinder Mode
    m_viewfinder = new ViewfinderWidget(m_engine.get(), m_stackedWidget);
    m_stackedWidget->addWidget(m_viewfinder);

    // Index 1: Hi-Fi Audio Deck Mode
    m_audioDeck = new AudioDeckWidget(m_engine.get(), m_stackedWidget);
    m_stackedWidget->addWidget(m_audioDeck);

    setCentralWidget(m_stackedWidget);
    setMode(UIMode::VideoViewfinder);
}

void MainWindow::setupConnections()
{
    // Mode switch connections
    connect(m_viewfinder, &ViewfinderWidget::switchModeRequested, this, &MainWindow::toggleMode);
    connect(m_audioDeck, &AudioDeckWidget::switchModeRequested, this, &MainWindow::toggleMode);
    connect(m_viewfinder, &ViewfinderWidget::fullscreenToggleRequested, this, &MainWindow::toggleFullscreen);

    // Engine signal connections
    connect(m_engine.get(), &Core::PlaybackEngine::metadataChanged, this, &MainWindow::onEngineMetadataChanged);
    connect(m_engine.get(), &Core::PlaybackEngine::errorOccurred, this, &MainWindow::onEngineErrorOccurred);
    connect(m_engine.get(), &Core::PlaybackEngine::mediaFinished, this, &MainWindow::onEngineMediaFinished);
}

void MainWindow::setupShortcuts()
{
    auto addAppShortcut = [this](const QKeySequence &seq, auto slotOrLambda) {
        auto *sc = new QShortcut(seq, this);
        sc->setContext(Qt::ApplicationShortcut);
        connect(sc, &QShortcut::activated, this, slotOrLambda);
    };

    // 1. Play / Pause
    addAppShortcut(QKeySequence(Qt::Key_Space), [this]() { if (m_engine) m_engine->togglePlayPause(); });
    addAppShortcut(QKeySequence(Qt::Key_K), [this]() { if (m_engine) m_engine->togglePlayPause(); });

    // 2. File Dialogs
    addAppShortcut(QKeySequence(Qt::CTRL | Qt::Key_O), [this]() { openFileDialog(); });
    addAppShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O), [this]() { openDirectoryDialog(); });

    // 3. Seeking
    addAppShortcut(QKeySequence(Qt::Key_Left), [this]() { if (m_engine) m_engine->seekRelative(-10000); });
    addAppShortcut(QKeySequence(Qt::Key_J), [this]() { if (m_engine) m_engine->seekRelative(-10000); });
    addAppShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Left), [this]() { if (m_engine) m_engine->seekRelative(-30000); });
    addAppShortcut(QKeySequence(Qt::Key_Right), [this]() { if (m_engine) m_engine->seekRelative(10000); });
    addAppShortcut(QKeySequence(Qt::Key_L), [this]() { if (m_engine) m_engine->seekRelative(10000); });
    addAppShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Right), [this]() { if (m_engine) m_engine->seekRelative(30000); });

    // 4. Frame Stepping
    addAppShortcut(QKeySequence(Qt::Key_Comma), [this]() { if (m_engine) m_engine->frameStep(-1); });
    addAppShortcut(QKeySequence(Qt::Key_Period), [this]() { if (m_engine) m_engine->frameStep(1); });

    // 5. Volume & Mute
    addAppShortcut(QKeySequence(Qt::Key_Up), [this]() { if (m_engine) m_engine->setVolume(std::min(100, m_engine->volume() + 5)); });
    addAppShortcut(QKeySequence(Qt::Key_Down), [this]() { if (m_engine) m_engine->setVolume(std::max(0, m_engine->volume() - 5)); });
    addAppShortcut(QKeySequence(Qt::Key_M), [this]() { if (m_engine) m_engine->setMuted(!m_engine->isMuted()); });

    // 6. Speed
    addAppShortcut(QKeySequence(Qt::Key_BracketLeft), [this]() { if (m_engine) m_engine->setSpeed(std::max(0.25, m_engine->speed() - 0.1)); });
    addAppShortcut(QKeySequence(Qt::Key_BracketRight), [this]() { if (m_engine) m_engine->setSpeed(std::min(4.0, m_engine->speed() + 0.1)); });
    addAppShortcut(QKeySequence(Qt::Key_Backspace), [this]() { if (m_engine) m_engine->setSpeed(1.0); });

    // 7. Fullscreen & Mode
    addAppShortcut(QKeySequence(Qt::Key_F), [this]() { toggleFullscreen(); });
    addAppShortcut(QKeySequence(Qt::Key_F11), [this]() { toggleFullscreen(); });
    addAppShortcut(QKeySequence(Qt::Key_Tab), [this]() { toggleMode(); });

    // 8. OSD & Reticles
    addAppShortcut(QKeySequence(Qt::Key_O), [this]() { if (m_viewfinder) m_viewfinder->toggleOsd(); });
    addAppShortcut(QKeySequence(Qt::Key_R), [this]() { if (m_viewfinder) m_viewfinder->toggleReticles(); });

    // 9. Quit
    addAppShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), [this]() { close(); });
}

void MainWindow::setMode(UIMode mode)
{
    m_currentMode = mode;
    m_stackedWidget->setCurrentIndex(static_cast<int>(mode));

    if (mode == UIMode::VideoViewfinder) {
        m_viewfinder->updateUIFromEngine();
    } else {
        m_audioDeck->updateUIFromEngine();
    }
}

void MainWindow::toggleMode()
{
    if (m_currentMode == UIMode::VideoViewfinder) {
        setMode(UIMode::HiFiAudioDeck);
    } else {
        setMode(UIMode::VideoViewfinder);
    }
}

void MainWindow::toggleFullscreen()
{
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
}

bool MainWindow::openMedia(const QString &filePath, bool autoPlay)
{
    if (filePath.isEmpty()) return false;

    QFileInfo fi(filePath);
    if (!fi.exists()) return false;

    QString ext = fi.suffix().toLower();

    // Sidecar subtitle / lyric handling
    if (ext == "lrc") {
        m_engine->loadExternalLrc(filePath);
        return true;
    } else if (ext == "srt" || ext == "ass" || ext == "ssa" || ext == "vtt") {
        m_engine->loadExternalSubtitle(filePath);
        return true;
    }

    // Auto-detect mode based on media format extension
    static const QStringList audioExts = {"mp3", "flac", "wav", "opus", "ogg", "m4a", "aac", "alac", "aiff", "dsd"};
    if (audioExts.contains(ext)) {
        setMode(UIMode::HiFiAudioDeck);
    } else {
        setMode(UIMode::VideoViewfinder);
    }

    // Add to playlist matrix queue
    m_audioDeck->playlistMatrix()->addItem(filePath, fi.baseName(), "", "", 0, ext.toUpper());
    m_audioDeck->playlistMatrix()->setCurrentIndex(m_audioDeck->playlistMatrix()->count() - 1);

    bool ok = m_engine->loadMedia(filePath, autoPlay);
    return ok;
}

void MainWindow::openDirectory(const QString &dirPath)
{
    if (dirPath.isEmpty()) return;

    QDir dir(dirPath);
    if (!dir.exists()) return;

    static const QStringList filters = {
        "*.mp4", "*.mkv", "*.webm", "*.avi", "*.mov", "*.ts", "*.flv",
        "*.mp3", "*.flac", "*.wav", "*.opus", "*.ogg", "*.m4a", "*.aac"
    };

    QDirIterator it(dirPath, filters, QDir::Files, QDirIterator::Subdirectories);
    bool first = true;
    while (it.hasNext()) {
        QString file = it.next();
        if (first) {
            openMedia(file, true);
            first = false;
        } else {
            QFileInfo fi(file);
            m_audioDeck->playlistMatrix()->addItem(file, fi.baseName(), "", "", 0, fi.suffix().toUpper());
        }
    }
}

void MainWindow::openFileDialog()
{
    QString filter = "Media Files (*.mp4 *.mkv *.webm *.avi *.mp3 *.flac *.wav *.opus *.ogg *.aac *.m4a *.srt *.ass *.vtt *.lrc);;All Files (*.*)";
    QString filePath = QFileDialog::getOpenFileName(this, "Open Media File", QString(), filter);
    if (!filePath.isEmpty()) {
        openMedia(filePath, true);
    }
}

void MainWindow::openDirectoryDialog()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, "Open Media Folder", QString());
    if (!dirPath.isEmpty()) {
        openDirectory(dirPath);
    }
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto *ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Tab) {
            toggleMode();
            event->accept();
            return true;
        }
    }
    return QMainWindow::event(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (!m_engine) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    int key = event->key();
    Qt::KeyboardModifiers mods = event->modifiers();

    // 1. Play / Pause (Space or K)
    if (key == Qt::Key_Space || key == Qt::Key_K) {
        m_engine->togglePlayPause();
        event->accept();
        return;
    }

    // 2. Seeking jumps
    if (key == Qt::Key_Left || key == Qt::Key_J) {
        qint64 delta = (mods & Qt::ShiftModifier) ? -30000 : -10000;
        m_engine->seekRelative(delta);
        event->accept();
        return;
    }
    if (key == Qt::Key_Right || key == Qt::Key_L) {
        qint64 delta = (mods & Qt::ShiftModifier) ? 30000 : 10000;
        m_engine->seekRelative(delta);
        event->accept();
        return;
    }

    // 3. Single frame stepping (< 1F / 1F >)
    if (key == Qt::Key_Comma) {
        m_engine->frameStep(-1);
        event->accept();
        return;
    }
    if (key == Qt::Key_Period) {
        m_engine->frameStep(1);
        event->accept();
        return;
    }

    // 4. Volume Control (Up / Down / Mute)
    if (key == Qt::Key_Up) {
        m_engine->setVolume(std::min(100, m_engine->volume() + 5));
        event->accept();
        return;
    }
    if (key == Qt::Key_Down) {
        m_engine->setVolume(std::max(0, m_engine->volume() - 5));
        event->accept();
        return;
    }
    if (key == Qt::Key_M) {
        m_engine->setMuted(!m_engine->isMuted());
        event->accept();
        return;
    }

    // 5. Playback Speed Rate ([ / ] / Backspace)
    if (key == Qt::Key_BracketLeft) {
        double currentSpeed = m_engine->speed();
        m_engine->setSpeed(std::max(0.25, currentSpeed - 0.1));
        event->accept();
        return;
    }
    if (key == Qt::Key_BracketRight) {
        double currentSpeed = m_engine->speed();
        m_engine->setSpeed(std::min(4.0, currentSpeed + 0.1));
        event->accept();
        return;
    }
    if (key == Qt::Key_Backspace) {
        m_engine->setSpeed(1.0);
        event->accept();
        return;
    }

    // 6. Fullscreen toggle (F or F11)
    if (key == Qt::Key_F || key == Qt::Key_F11) {
        toggleFullscreen();
        event->accept();
        return;
    }

    // 7. Video OSD & Reticle toggles (O / R)
    if (key == Qt::Key_O) {
        m_viewfinder->toggleOsd();
        event->accept();
        return;
    }
    if (key == Qt::Key_R) {
        m_viewfinder->toggleReticles();
        event->accept();
        return;
    }

    // 8. Mode Switch (Tab)
    if (key == Qt::Key_Tab) {
        toggleMode();
        event->accept();
        return;
    }

    // 9. File I/O shortcuts
    if ((mods & Qt::ControlModifier) && key == Qt::Key_O) {
        if (mods & Qt::ShiftModifier) {
            openDirectoryDialog();
        } else {
            openFileDialog();
        }
        event->accept();
        return;
    }

    // 10. Quit (Ctrl+Q)
    if ((mods & Qt::ControlModifier) && key == Qt::Key_Q) {
        close();
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mime = event->mimeData();
    if (!mime->hasUrls()) return;

    QList<QUrl> urls = mime->urls();
    bool first = true;

    for (const QUrl &url : urls) {
        if (!url.isLocalFile()) continue;
        QString localPath = url.toLocalFile();
        QFileInfo fi(localPath);

        if (fi.isDir()) {
            openDirectory(localPath);
        } else if (fi.isFile()) {
            if (first) {
                openMedia(localPath, true);
                first = false;
            } else {
                QString ext = fi.suffix().toLower();
                if (ext == "lrc") {
                    m_engine->loadExternalLrc(localPath);
                } else if (ext == "srt" || ext == "ass" || ext == "vtt") {
                    m_engine->loadExternalSubtitle(localPath);
                } else {
                    m_audioDeck->playlistMatrix()->addItem(localPath, fi.baseName(), "", "", 0, ext.toUpper());
                }
            }
        }
    }

    event->acceptProposedAction();
}

void MainWindow::onEngineMetadataChanged(const Core::MediaMetadata &meta)
{
    QString title = meta.title.isEmpty() ? QFileInfo(meta.url).fileName() : meta.title;
    if (title.isEmpty()) {
        setWindowTitle("Penguin // Tactile Digital Brutalist Media Player");
    } else {
        setWindowTitle(QString("Penguin // %1").arg(title));
    }
}

void MainWindow::onEngineErrorOccurred(const QString &errorMessage)
{
    qWarning() << "Penguin Engine Error:" << errorMessage;
}

void MainWindow::onEngineMediaFinished()
{
    if (m_playlistMgr) {
        m_playlistMgr->next();
    } else if (m_audioDeck) {
        m_audioDeck->nextTrack();
    }
}

void MainWindow::setStatePersistence(Library::StatePersistence *persistence)
{
    m_persistence = persistence;
    if (m_persistence) {
        restoreState();
    }
}

void MainWindow::setPlaylistManager(Library::PlaylistManager *playlistMgr)
{
    m_playlistMgr = playlistMgr;
    if (m_playlistMgr) {
        connect(m_playlistMgr, &Library::PlaylistManager::currentTrackChanged, this, [this](int, const UI::PlaylistItem &item) {
            openMedia(item.filePath, true);
        });
    }
}

void MainWindow::saveState()
{
    if (!m_persistence) return;
    m_persistence->saveWindowState(this);

    QVector<double> gains(10, 0.0);
    for (int i = 0; i < 10; ++i) {
        gains[i] = m_engine->equalizerBand(i);
    }
    m_persistence->saveAudioSettings(
        m_engine->volume(),
        m_engine->isMuted(),
        m_engine->speed(),
        m_engine->currentEqualizerPreset(),
        gains
    );

    m_persistence->saveUIMode(static_cast<int>(m_currentMode));
    m_persistence->saveTelemetryPreferences(m_viewfinder->isOsdVisible(), m_viewfinder->isReticlesVisible());

    if (m_audioDeck && m_audioDeck->playlistMatrix()) {
        m_persistence->saveCurrentPlaylist(
            m_audioDeck->playlistMatrix()->items(),
            m_audioDeck->playlistMatrix()->currentIndex()
        );
    }

    if (m_engine && !m_engine->currentUri().isEmpty()) {
        const auto &meta = m_engine->metadata();
        m_persistence->recordPlayback(
            m_engine->currentUri(),
            meta.title,
            meta.artist,
            meta.album,
            m_engine->durationMs(),
            m_engine->positionMs(),
            (m_currentMode == UIMode::HiFiAudioDeck) ? "audio" : "video"
        );
    }
}

void MainWindow::restoreState()
{
    if (!m_persistence) return;
    m_persistence->restoreWindowState(this);

    int vol = 85;
    bool muted = false;
    double spd = 1.0;
    QString preset = "Flat";
    QVector<double> gains(10, 0.0);
    m_persistence->restoreAudioSettings(vol, muted, spd, preset, gains);

    m_engine->setVolume(vol);
    m_engine->setMuted(muted);
    m_engine->setSpeed(spd);
    if (!preset.isEmpty()) {
        m_engine->setEqualizerPreset(preset);
    }
    for (int i = 0; i < gains.size() && i < 10; ++i) {
        m_engine->setEqualizerBand(i, gains[i]);
    }

    int mode = m_persistence->restoreUIMode(static_cast<int>(UIMode::VideoViewfinder));
    setMode(static_cast<UIMode>(mode));

    bool osd = true, reticles = true;
    m_persistence->restoreTelemetryPreferences(osd, reticles);
    if (m_viewfinder) {
        if (m_viewfinder->isOsdVisible() != osd) m_viewfinder->toggleOsd();
        if (m_viewfinder->isReticlesVisible() != reticles) m_viewfinder->toggleReticles();
    }

    int curIdx = -1;
    QList<PlaylistItem> playlist = m_persistence->restoreCurrentPlaylist(curIdx);
    if (!playlist.isEmpty() && m_audioDeck && m_audioDeck->playlistMatrix()) {
        m_audioDeck->playlistMatrix()->clearPlaylist();
        m_audioDeck->playlistMatrix()->addItems(playlist);
        m_audioDeck->playlistMatrix()->setCurrentIndex(curIdx);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveState();
    QMainWindow::closeEvent(event);
}

} // namespace UI
} // namespace Penguin
