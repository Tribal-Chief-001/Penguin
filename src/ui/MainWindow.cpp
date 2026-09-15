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
#include <QInputDialog>
#include <QStatusBar>
#include <QDialog>
#include <QFormLayout>
#include <QLabel>
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

    // 2. File & Stream Dialogs
    addAppShortcut(QKeySequence(Qt::CTRL | Qt::Key_O), [this]() { openFileDialog(); });
    addAppShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O), [this]() { openDirectoryDialog(); });
    addAppShortcut(QKeySequence(Qt::CTRL | Qt::Key_U), [this]() { openUrlDialog(); });

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
    addAppShortcut(QKeySequence(Qt::Key_BracketLeft), [this]() { setLoopPointA(); });
    addAppShortcut(QKeySequence(Qt::Key_BracketRight), [this]() { setLoopPointB(); });
    addAppShortcut(QKeySequence(Qt::Key_Backslash), [this]() { clearLoop(); });
    addAppShortcut(QKeySequence(Qt::Key_Backspace), [this]() { if (m_engine) m_engine->setSpeed(1.0); });

    // 7. Subtitle & Audio Delays
    addAppShortcut(QKeySequence(Qt::Key_Z), [this]() { adjustSubtitleDelay(-50); });
    addAppShortcut(QKeySequence(Qt::Key_X), [this]() { adjustSubtitleDelay(50); });
    addAppShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Z), [this]() { adjustAudioDelay(-50); });
    addAppShortcut(QKeySequence(Qt::SHIFT | Qt::Key_X), [this]() { adjustAudioDelay(50); });

    // 8. Visual & DSP Toggles
    addAppShortcut(QKeySequence(Qt::Key_N), [this]() { toggleNightMode(); });
    addAppShortcut(QKeySequence(Qt::Key_C), [this]() { toggleCrossfeed(); });
    addAppShortcut(QKeySequence(Qt::Key_D), [this]() { toggleDeband(); });
    addAppShortcut(QKeySequence(Qt::Key_A), [this]() { cycleAspectRatio(); });
    addAppShortcut(QKeySequence(Qt::CTRL | Qt::Key_E), [this]() { openVideoEqualizerDialog(); });
    addAppShortcut(QKeySequence(Qt::Key_T), [this]() { togglePipMode(); });
    addAppShortcut(QKeySequence(Qt::Key_V), [this]() { cycleSecondarySubtitle(); });

    // 9. Pitch Shifting (Musical Semitones)
    addAppShortcut(QKeySequence(Qt::ALT | Qt::Key_Up), [this]() { adjustPitch(1.0); });
    addAppShortcut(QKeySequence(Qt::ALT | Qt::Key_Down), [this]() { adjustPitch(-1.0); });
    addAppShortcut(QKeySequence(Qt::ALT | Qt::Key_0), [this]() { adjustPitch(0.0); });

    // 10. Chapters & Bookmarks Navigation
    addAppShortcut(QKeySequence(Qt::Key_PageDown), [this]() { nextChapter(); });
    addAppShortcut(QKeySequence(Qt::Key_PageUp), [this]() { previousChapter(); });
    addAppShortcut(QKeySequence(Qt::Key_B), [this]() { addBookmark(); });
    addAppShortcut(QKeySequence(Qt::SHIFT | Qt::Key_B), [this]() { nextBookmark(); });

    // 11. Forensic Screenshots
    addAppShortcut(QKeySequence(Qt::Key_S), [this]() { takeScreenshot(false); });
    addAppShortcut(QKeySequence(Qt::SHIFT | Qt::Key_S), [this]() { takeScreenshot(true); });

    // 12. Fullscreen & Mode
    addAppShortcut(QKeySequence(Qt::Key_F), [this]() { toggleFullscreen(); });
    addAppShortcut(QKeySequence(Qt::Key_F11), [this]() { toggleFullscreen(); });
    addAppShortcut(QKeySequence(Qt::Key_Tab), [this]() { toggleMode(); });

    // 13. OSD & Reticles
    addAppShortcut(QKeySequence(Qt::Key_O), [this]() { if (m_viewfinder) m_viewfinder->toggleOsd(); });
    addAppShortcut(QKeySequence(Qt::Key_R), [this]() { if (m_viewfinder) m_viewfinder->toggleReticles(); });

    // 14. Quit
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

    bool isNetworkUrl = filePath.startsWith("http://", Qt::CaseInsensitive) ||
                        filePath.startsWith("https://", Qt::CaseInsensitive) ||
                        filePath.startsWith("rtmp://", Qt::CaseInsensitive) ||
                        filePath.startsWith("ytdl://", Qt::CaseInsensitive);

    QString ext;
    QString title;

    if (isNetworkUrl) {
        ext = "STREAM";
        title = filePath;
        setMode(UIMode::VideoViewfinder);
    } else {
        QFileInfo fi(filePath);
        if (!fi.exists()) return false;

        ext = fi.suffix().toLower();
        title = fi.baseName();

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
    }

    // Add to playlist manager / playlist matrix if not already there or update index
    if (m_playlistMgr) {
        bool found = false;
        const auto &items = m_playlistMgr->items();
        for (int i = 0; i < items.size(); ++i) {
            if (items[i].filePath == filePath) {
                m_playlistMgr->setCurrentIndex(i);
                found = true;
                break;
            }
        }
        if (!found) {
            m_playlistMgr->addItem(filePath, title, "", "", 0, isNetworkUrl ? "STREAM" : ext.toUpper());
            m_playlistMgr->setCurrentIndex(m_playlistMgr->count() - 1);
        }
    } else if (m_audioDeck && m_audioDeck->playlistMatrix()) {
        bool foundInMatrix = false;
        const auto &matrixItems = m_audioDeck->playlistMatrix()->items();
        for (int i = 0; i < matrixItems.size(); ++i) {
            if (matrixItems[i].filePath == filePath) {
                m_audioDeck->playlistMatrix()->setCurrentIndex(i);
                foundInMatrix = true;
                break;
            }
        }
        if (!foundInMatrix) {
            m_audioDeck->playlistMatrix()->addItem(filePath, title, "", "", 0, isNetworkUrl ? "STREAM" : ext.toUpper());
            m_audioDeck->playlistMatrix()->setCurrentIndex(m_audioDeck->playlistMatrix()->count() - 1);
        }
    }

    bool ok = m_engine->loadMedia(filePath, autoPlay);
    return ok;
}

void MainWindow::openDirectory(const QString &dirPath)
{
    if (dirPath.isEmpty()) return;

    QDir dir(dirPath);
    if (!dir.exists()) return;

    if (m_playlistMgr) {
        int initialCount = m_playlistMgr->count();
        m_playlistMgr->addDirectory(dirPath, true);
        if (m_playlistMgr->count() > initialCount) {
            m_playlistMgr->setCurrentIndex(initialCount);
        }
        return;
    }

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
            if (m_audioDeck && m_audioDeck->playlistMatrix()) {
                m_audioDeck->playlistMatrix()->addItem(file, fi.baseName(), "", "", 0, fi.suffix().toUpper());
            }
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

void MainWindow::openUrlDialog()
{
    bool ok = false;
    QString url = QInputDialog::getText(this, "OPEN STREAM URL", "Enter Network URL / YouTube / Twitch / HLS Stream:", QLineEdit::Normal, "", &ok);
    if (ok && !url.trimmed().isEmpty()) {
        openMedia(url.trimmed(), true);
        showOsdMessage("STREAM INGESTED // " + url.trimmed());
    }
}

void MainWindow::takeScreenshot(bool includeSubtitles)
{
    if (!m_engine) return;
    bool success = m_engine->takeScreenshot(QString(), includeSubtitles);
    if (success) {
        showOsdMessage(QString("SCREENSHOT SAVED [%1]").arg(includeSubtitles ? "WITH SUBTITLES" : "CLEAN FRAME"));
    }
}

void MainWindow::setLoopPointA()
{
    if (!m_engine) return;
    m_engine->setLoopPointA();
    showOsdMessage(QString("LOOP POINT [A] SET // %1").arg(m_engine->smptePosition()));
}

void MainWindow::setLoopPointB()
{
    if (!m_engine) return;
    m_engine->setLoopPointB();
    showOsdMessage(QString("LOOP POINT [B] SET // %1 (ACTIVE)").arg(m_engine->smptePosition()));
}

void MainWindow::clearLoop()
{
    if (!m_engine) return;
    m_engine->clearLoop();
    showOsdMessage("A-B LOOP CLEARED");
}

void MainWindow::adjustSubtitleDelay(int deltaMs)
{
    if (!m_engine) return;
    m_engine->adjustSubtitleDelayMs(deltaMs);
    showOsdMessage(QString("SUBTITLE OFFSET: %1%2 ms").arg(m_engine->subtitleDelayMs() > 0 ? "+" : "").arg(m_engine->subtitleDelayMs()));
}

void MainWindow::adjustAudioDelay(int deltaMs)
{
    if (!m_engine) return;
    m_engine->adjustAudioDelayMs(deltaMs);
    showOsdMessage(QString("AUDIO OFFSET: %1%2 ms").arg(m_engine->audioDelayMs() > 0 ? "+" : "").arg(m_engine->audioDelayMs()));
}

void MainWindow::toggleNightMode()
{
    if (!m_engine) return;
    bool newState = !m_engine->isNightMode();
    m_engine->setNightMode(newState);
    showOsdMessage(QString("NIGHT COMPRESSOR: %1").arg(newState ? "ACTIVE [ENGAGED]" : "BYPASSED"));
}

void MainWindow::toggleDeband()
{
    if (!m_engine) return;
    bool newState = !m_engine->isDebandEnabled();
    m_engine->setDebandEnabled(newState);
    showOsdMessage(QString("DEBAND DITHERING: %1").arg(newState ? "ENABLED" : "DISABLED"));
}

void MainWindow::cycleAspectRatio()
{
    if (!m_engine) return;
    QString current = m_engine->aspectRatio();
    QString next = "auto";
    if (current == "auto" || current == "-1") next = "16:9";
    else if (current == "16:9") next = "4:3";
    else if (current == "4:3") next = "21:9";
    else if (current == "21:9") next = "2.35:1";
    else next = "auto";

    m_engine->setAspectRatio(next);
    showOsdMessage(QString("ASPECT RATIO: %1").arg(next.toUpper()));
}

void MainWindow::toggleCrossfeed()
{
    if (!m_engine) return;
    bool newState = !m_engine->isCrossfeedEnabled();
    m_engine->setCrossfeedEnabled(newState);
    showOsdMessage(QString("HEADPHONE CROSSFEED (BS2B): %1").arg(newState ? "ACTIVE [ENGAGED]" : "BYPASSED"));
}

void MainWindow::togglePipMode()
{
    m_isPip = !m_isPip;
    if (m_isPip) {
        m_savedGeometry = geometry();
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        if (m_viewfinder) {
            m_viewfinder->setOsdVisible(false);
            m_viewfinder->setReticlesVisible(false);
        }
        resize(400, 225);
        show();
        showOsdMessage("PICTURE-IN-PICTURE (PIP) // PINNED ON TOP");
    } else {
        setWindowFlags(Qt::Window);
        if (m_savedGeometry.isValid()) {
            setGeometry(m_savedGeometry);
        }
        show();
        showOsdMessage("PICTURE-IN-PICTURE (PIP) // RESTORED");
    }
}

void MainWindow::cycleSecondarySubtitle()
{
    if (!m_engine) return;
    m_engine->cycleSecondarySubtitle();
    int sid = m_engine->selectedSecondarySubtitleTrackId();
    showOsdMessage(QString("SECONDARY SUBTITLE: %1").arg(sid > 0 ? QString("TRACK %1").arg(sid) : "OFF"));
}

void MainWindow::addBookmark()
{
    if (!m_engine) return;
    qint64 pos = m_engine->positionMs();
    m_engine->addBookmark(pos);
    showOsdMessage(QString("BOOKMARK ADDED @ %1").arg(Core::TimecodeFormatter::formatTimecode(pos, 30.0)));
}

void MainWindow::nextBookmark()
{
    if (!m_engine) return;
    m_engine->nextBookmark();
    qint64 pos = m_engine->positionMs();
    showOsdMessage(QString("JUMP TO BOOKMARK @ %1").arg(Core::TimecodeFormatter::formatTimecode(pos, 30.0)));
}

void MainWindow::adjustPitch(double deltaSemitones)
{
    if (!m_engine) return;
    if (qFuzzyIsNull(deltaSemitones)) {
        m_engine->setPitch(0.0);
        showOsdMessage("PITCH SHIFT // RESET TO NORMAL (0 ST)");
    } else {
        m_engine->adjustPitch(deltaSemitones);
        double p = m_engine->pitch();
        showOsdMessage(QString("PITCH SHIFT // %1%2 SEMITONES").arg(p > 0 ? "+" : "").arg(p, 0, 'f', 1));
    }
}

void MainWindow::nextChapter()
{
    if (!m_engine) return;
    m_engine->nextChapter();
    QString title = m_engine->currentChapterTitle();
    showOsdMessage(QString("CHAPTER NEXT // %1").arg(title.isEmpty() ? "NEXT" : title));
}

void MainWindow::previousChapter()
{
    if (!m_engine) return;
    m_engine->previousChapter();
    QString title = m_engine->currentChapterTitle();
    showOsdMessage(QString("CHAPTER PREV // %1").arg(title.isEmpty() ? "PREV" : title));
}

void MainWindow::openVideoEqualizerDialog()
{
    if (!m_engine) return;
    auto *dlg = new QDialog(this);
    dlg->setWindowTitle("VIDEO EQUALIZER // COLOR SCIENCE");
    dlg->setFixedWidth(360);
    dlg->setStyleSheet(BrutalistTheme::videoEqualizerDialogStyleSheet());

    auto *layout = new QVBoxLayout(dlg);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    auto *header = new QLabel("VIDEO COLOR EQUALIZER", dlg);
    header->setFont(BrutalistTheme::monospaceFont(10, QFont::Bold));
    header->setStyleSheet("color: #FF4400; border-bottom: 1px solid #1E1E24; padding-bottom: 6px;");
    layout->addWidget(header);

    auto makeSliderRow = [dlg](const QString &label, int currentVal, auto callback) {
        auto *row = new QHBoxLayout();
        auto *lbl = new QLabel(label, dlg);
        lbl->setFont(BrutalistTheme::monospaceFont(8, QFont::Normal));
        lbl->setFixedWidth(90);
        lbl->setStyleSheet("color: #E2E2EA;");

        auto *valLbl = new QLabel(QString("%1%2").arg(currentVal > 0 ? "+" : "").arg(currentVal), dlg);
        valLbl->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
        valLbl->setFixedWidth(40);
        valLbl->setStyleSheet("color: #00E5FF;");

        auto *slider = new QSlider(Qt::Horizontal, dlg);
        slider->setRange(-100, 100);
        slider->setValue(currentVal);

        QObject::connect(slider, &QSlider::valueChanged, dlg, [valLbl, callback](int v) {
            valLbl->setText(QString("%1%2").arg(v > 0 ? "+" : "").arg(v));
            callback(v);
        });

        row->addWidget(lbl);
        row->addWidget(slider, 1);
        row->addWidget(valLbl);
        return row;
    };

    layout->addLayout(makeSliderRow("BRIGHTNESS", m_engine->backend()->brightness(), [this](int v) { m_engine->setBrightness(v); }));
    layout->addLayout(makeSliderRow("CONTRAST", m_engine->backend()->contrast(), [this](int v) { m_engine->setContrast(v); }));
    layout->addLayout(makeSliderRow("GAMMA", m_engine->backend()->gamma(), [this](int v) { m_engine->setGamma(v); }));
    layout->addLayout(makeSliderRow("SATURATION", m_engine->backend()->saturation(), [this](int v) { m_engine->setSaturation(v); }));
    layout->addLayout(makeSliderRow("HUE", m_engine->backend()->hue(), [this](int v) { m_engine->setHue(v); }));

    auto *btnRow = new QHBoxLayout();
    auto *resetBtn = new QPushButton("FLAT RESET", dlg);
    resetBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    resetBtn->setStyleSheet(BrutalistTheme::pillButtonStyleSheet());
    connect(resetBtn, &QPushButton::clicked, dlg, [this, dlg]() {
        m_engine->resetVideoEqualizer();
        dlg->close();
        showOsdMessage("VIDEO EQUALIZER RESET TO FLAT");
    });

    auto *closeBtn = new QPushButton("CLOSE", dlg);
    closeBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    closeBtn->setStyleSheet(BrutalistTheme::accentLimeButtonStyleSheet());
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    btnRow->addWidget(resetBtn);
    btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);

    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::showOsdMessage(const QString &message, int durationMs)
{
    statusBar()->showMessage(message, durationMs);
    if (m_viewfinder && m_viewfinder->videoSurface()) {
        m_viewfinder->videoSurface()->setMediaTitle(message);
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
        if (url.isLocalFile()) {
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
                        if (m_playlistMgr) {
                            m_playlistMgr->addItem(localPath, fi.baseName(), "", "", 0, ext.toUpper());
                        } else if (m_audioDeck && m_audioDeck->playlistMatrix()) {
                            m_audioDeck->playlistMatrix()->addItem(localPath, fi.baseName(), "", "", 0, ext.toUpper());
                        }
                    }
                }
            }
        } else {
            QString urlStr = url.toString();
            if (urlStr.startsWith("http://", Qt::CaseInsensitive) ||
                urlStr.startsWith("https://", Qt::CaseInsensitive) ||
                urlStr.startsWith("rtmp://", Qt::CaseInsensitive) ||
                urlStr.startsWith("ytdl://", Qt::CaseInsensitive)) {
                if (first) {
                    openMedia(urlStr, true);
                    first = false;
                } else {
                    if (m_playlistMgr) {
                        m_playlistMgr->addItem(urlStr, urlStr, "", "", 0, "STREAM");
                    } else if (m_audioDeck && m_audioDeck->playlistMatrix()) {
                        m_audioDeck->playlistMatrix()->addItem(urlStr, urlStr, "", "", 0, "STREAM");
                    }
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
    if (m_playlistMgr && m_playlistMgr->count() > 0) {
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
    if (m_audioDeck) {
        m_audioDeck->setPlaylistManager(m_playlistMgr);
    }
    if (m_playlistMgr) {
        connect(m_playlistMgr, &Library::PlaylistManager::currentTrackChanged, this, [this](int, const UI::PlaylistItem &item) {
            openMedia(item.filePath, true);
        });
        connect(m_playlistMgr, &Library::PlaylistManager::playlistUpdated, this, [this]() {
            if (m_audioDeck && m_audioDeck->playlistMatrix()) {
                m_audioDeck->playlistMatrix()->setItems(m_playlistMgr->items(), m_playlistMgr->currentIndex());
            }
        });
        if (m_audioDeck && m_audioDeck->playlistMatrix()) {
            m_audioDeck->playlistMatrix()->setItems(m_playlistMgr->items(), m_playlistMgr->currentIndex());
        }
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
