#include "ViewfinderWidget.h"
#include "BrutalistTheme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QFileInfo>
#include <QMouseEvent>
#include <cmath>

namespace Penguin {
namespace UI {

// ----------------------------------------------------------------------------
// VideoSurfaceWidget Implementation
// ----------------------------------------------------------------------------

VideoSurfaceWidget::VideoSurfaceWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_NativeWindow, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);

    m_singleClickTimer.setSingleShot(true);
    m_singleClickTimer.setInterval(250);
    connect(&m_singleClickTimer, &QTimer::timeout, this, &VideoSurfaceWidget::clicked);
}

void VideoSurfaceWidget::setPlaybackEngine(Core::PlaybackEngine *engine)
{
    m_engine = engine;
    attachWindowId();
}

void VideoSurfaceWidget::attachWindowId()
{
    if (m_engine && m_engine->backend()) {
        m_engine->setWindowId((int64_t)winId());
    }
}

void VideoSurfaceWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    attachWindowId();
}

void VideoSurfaceWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    attachWindowId();
}

void VideoSurfaceWidget::paintEvent(QPaintEvent * /*event*/)
{
    bool hasVideo = m_engine && m_engine->backend() && m_engine->backend()->metadata().hasVideo && m_engine->playbackState() != Core::PlaybackState::Stopped;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    int vw = width();
    int vh = height();
    int cx = vw / 2;
    int cy = vh / 2;

    if (!hasVideo) {
        // Deep Obsidian canvas
        p.fillRect(rect(), BrutalistTheme::BG_DEEP_OBSIDIAN);

        // Technical background grid when no video
        p.setPen(QPen(QColor(30, 30, 36, 60), 1));
        for (int x = 0; x < vw; x += 32) {
            p.drawLine(x, 0, x, vh);
        }
        for (int y = 0; y < vh; y += 32) {
            p.drawLine(0, y, vw, y);
        }
    }

    // Border around video frame
    p.setPen(QPen(BrutalistTheme::GRID_STRUCTURAL_BORDER, 1));
    p.drawRect(rect().adjusted(0, 0, -1, -1));

    // Safe-Area Reticles
    if (m_showReticles) {
        // 1. Action-Safe Area (90% boundary: dashed cyan at 40% opacity)
        int actionW = static_cast<int>(vw * 0.90);
        int actionH = static_cast<int>(vh * 0.90);
        QRect actionRect(cx - actionW / 2, cy - actionH / 2, actionW, actionH);

        QPen actionPen(QColor(0, 229, 255, 100), 1, Qt::DashLine);
        p.setPen(actionPen);
        p.drawRect(actionRect);

        // Action safe corner marks (L-shaped)
        int cornerLen = 12;
        p.drawLine(actionRect.left(), actionRect.top(), actionRect.left() + cornerLen, actionRect.top());
        p.drawLine(actionRect.left(), actionRect.top(), actionRect.left(), actionRect.top() + cornerLen);
        p.drawLine(actionRect.right(), actionRect.top(), actionRect.right() - cornerLen, actionRect.top());
        p.drawLine(actionRect.right(), actionRect.top(), actionRect.right(), actionRect.top() + cornerLen);
        p.drawLine(actionRect.left(), actionRect.bottom(), actionRect.left() + cornerLen, actionRect.bottom());
        p.drawLine(actionRect.left(), actionRect.bottom(), actionRect.left(), actionRect.bottom() - cornerLen);
        p.drawLine(actionRect.right(), actionRect.bottom(), actionRect.right() - cornerLen, actionRect.bottom());
        p.drawLine(actionRect.right(), actionRect.bottom(), actionRect.right(), actionRect.bottom() - cornerLen);

        // 2. Title-Safe Area (80% boundary: solid cyan at 50% opacity)
        int titleW = static_cast<int>(vw * 0.80);
        int titleH = static_cast<int>(vh * 0.80);
        QRect titleRect(cx - titleW / 2, cy - titleH / 2, titleW, titleH);

        QPen titlePen(QColor(0, 229, 255, 128), 1, Qt::SolidLine);
        p.setPen(titlePen);
        p.drawRect(titleRect);

        // 3. Center Crosshair (+) with 16px arms
        int arm = 16;
        p.setPen(QPen(BrutalistTheme::ACCENT_TELEMETRY_CYAN, 1));
        p.drawLine(cx - arm, cy, cx + arm, cy);
        p.drawLine(cx, cy - arm, cx, cy + arm);
        p.drawEllipse(QPoint(cx, cy), 4, 4);

        // Reticle Labels
        p.setFont(BrutalistTheme::monospaceFont(7, QFont::Normal));
        p.setPen(QColor(0, 229, 255, 140));
        p.drawText(actionRect.left() + 4, actionRect.top() + 10, "ACTION SAFE 90%");
        p.drawText(titleRect.left() + 4, titleRect.top() + 10, "TITLE SAFE 80%");
    }

    // Media Title Watermark when idling
    if (!hasVideo && !m_title.isEmpty()) {
        p.setFont(BrutalistTheme::monospaceFont(10, QFont::Bold));
        p.setPen(QColor(255, 255, 255, 80));
        p.drawText(rect().adjusted(16, 16, -16, -16), Qt::AlignBottom | Qt::AlignRight, m_title);
    }
}

void VideoSurfaceWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_singleClickTimer.start(250);
    }
    QWidget::mousePressEvent(event);
}

void VideoSurfaceWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_singleClickTimer.stop();
        emit doubleClicked();
    }
    QWidget::mouseDoubleClickEvent(event);
}

void VideoSurfaceWidget::mouseMoveEvent(QMouseEvent *event)
{
    emit mouseMoved();
    QWidget::mouseMoveEvent(event);
}

void VideoSurfaceWidget::wheelEvent(QWheelEvent *event)
{
    if (!m_engine) {
        QWidget::wheelEvent(event);
        return;
    }

    int numDegrees = event->angleDelta().y() / 8;
    int numSteps = numDegrees / 15;
    if (numSteps == 0 && numDegrees != 0) {
        numSteps = (numDegrees > 0) ? 1 : -1;
    }

    Qt::KeyboardModifiers mods = event->modifiers();
    if (mods & Qt::ControlModifier) {
        // Ctrl + Scroll = +/-10s seek
        qint64 seekMs = numSteps * 10000;
        m_engine->seekRelative(seekMs);
    } else if (mods & Qt::ShiftModifier) {
        // Shift + Scroll = single frame step
        m_engine->frameStep(numSteps > 0 ? 1 : -1);
    } else {
        // Vertical scroll = volume +/- 2%
        int curVol = m_engine->volume();
        int newVol = std::clamp(curVol + numSteps * 2, 0, 100);
        m_engine->setVolume(newVol);
    }

    event->accept();
}

// ----------------------------------------------------------------------------
// ViewfinderWidget Implementation
// ----------------------------------------------------------------------------

ViewfinderWidget::ViewfinderWidget(Core::PlaybackEngine *engine, QWidget *parent)
    : QWidget(parent)
    , m_engine(engine)
{
    setMouseTracking(true);
    m_autohideTimer.setInterval(2500);
    m_autohideTimer.setSingleShot(true);
    connect(&m_autohideTimer, &QTimer::timeout, this, &ViewfinderWidget::onAutohideTimeout);

    setupUI();
    if (m_engine) {
        connectEngineSignals();
        updateUIFromEngine();
    }
}

void ViewfinderWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 1. Top Header Bar
    m_headerWidget = new QWidget(this);
    m_headerWidget->setFixedHeight(32);
    m_headerWidget->setStyleSheet("background-color: #0B0B0E; border-bottom: 1px solid #1E1E24;");
    auto *headerLayout = new QHBoxLayout(m_headerWidget);
    headerLayout->setContentsMargins(8, 0, 8, 0);
    headerLayout->setSpacing(8);

    auto *modeTag = new QLabel("[MODE: VIDEO VIEWFINDER]", m_headerWidget);
    modeTag->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    modeTag->setStyleSheet("color: #FF4400; border: none; background: transparent;");
    headerLayout->addWidget(modeTag);

    m_titleLabel = new QLabel("NO MEDIA LOADED", m_headerWidget);
    m_titleLabel->setFont(BrutalistTheme::sansFont(9, QFont::Bold));
    m_titleLabel->setStyleSheet("color: #FFFFFF; border: none; background: transparent;");
    headerLayout->addWidget(m_titleLabel, 1);

    m_reticleToggleBtn = new QPushButton("RETICLE [R]", m_headerWidget);
    m_reticleToggleBtn->setCheckable(true);
    m_reticleToggleBtn->setChecked(true);
    m_reticleToggleBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Normal));
    connect(m_reticleToggleBtn, &QPushButton::clicked, this, &ViewfinderWidget::toggleReticles);
    headerLayout->addWidget(m_reticleToggleBtn);

    m_osdToggleBtn = new QPushButton("OSD [O]", m_headerWidget);
    m_osdToggleBtn->setCheckable(true);
    m_osdToggleBtn->setChecked(true);
    m_osdToggleBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Normal));
    connect(m_osdToggleBtn, &QPushButton::clicked, this, &ViewfinderWidget::toggleOsd);
    headerLayout->addWidget(m_osdToggleBtn);

    m_fsToggleBtn = new QPushButton("FULLSCREEN [F]", m_headerWidget);
    m_fsToggleBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Normal));
    connect(m_fsToggleBtn, &QPushButton::clicked, this, &ViewfinderWidget::fullscreenToggleRequested);
    headerLayout->addWidget(m_fsToggleBtn);

    m_switchModeBtn = new QPushButton("AUDIO DECK [TAB]", m_headerWidget);
    m_switchModeBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_switchModeBtn->setStyleSheet(BrutalistTheme::accentLimeButtonStyleSheet());
    connect(m_switchModeBtn, &QPushButton::clicked, this, &ViewfinderWidget::switchModeRequested);
    headerLayout->addWidget(m_switchModeBtn);

    mainLayout->addWidget(m_headerWidget);

    // 2. Video Viewport Surface & Diagnostics HUD Container
    auto *viewportContainer = new QWidget(this);
    viewportContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *viewportLayout = new QVBoxLayout(viewportContainer);
    viewportLayout->setContentsMargins(0, 0, 0, 0);
    viewportLayout->setSpacing(0);

    m_videoSurface = new VideoSurfaceWidget(viewportContainer);
    m_videoSurface->setPlaybackEngine(m_engine);
    connect(m_videoSurface, &VideoSurfaceWidget::clicked, this, &ViewfinderWidget::onPlayPauseClicked);
    connect(m_videoSurface, &VideoSurfaceWidget::doubleClicked, this, &ViewfinderWidget::fullscreenToggleRequested);
    viewportLayout->addWidget(m_videoSurface);

    // Diagnostics HUD positioned inside viewport container
    m_hud = new DiagnosticsHUDWidget(m_videoSurface);
    m_hud->move(12, 12);
    m_hud->resize(360, 100);

    mainLayout->addWidget(viewportContainer, 1);

    // 3. Mechanical Tick Scrubber
    m_scrubber = new TickScrubberWidget(this);
    m_scrubber->setFocusPolicy(Qt::NoFocus);
    mainLayout->addWidget(m_scrubber);

    // 4. Tactile Bottom Control Dock
    m_dockWidget = new QWidget(this);
    m_dockWidget->setFixedHeight(44);
    m_dockWidget->setStyleSheet("background-color: #0B0B0E; border-top: 1px solid #1E1E24;");
    auto *dockLayout = new QHBoxLayout(m_dockWidget);
    dockLayout->setContentsMargins(8, 4, 8, 4);
    dockLayout->setSpacing(6);

    // Transport buttons
    m_stepBackBtn = new QPushButton("|< 1F", m_dockWidget);
    m_stepBackBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_stepBackBtn->setToolTip("Step Backward 1 Frame (,)");
    m_stepBackBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_stepBackBtn, &QPushButton::clicked, this, &ViewfinderWidget::onStepBackwardClicked);
    dockLayout->addWidget(m_stepBackBtn);

    m_jumpBackBtn = new QPushButton("-10s", m_dockWidget);
    m_jumpBackBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_jumpBackBtn->setToolTip("Jump Backward 10 Seconds (Left Arrow)");
    m_jumpBackBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_jumpBackBtn, &QPushButton::clicked, this, &ViewfinderWidget::onJumpBackwardClicked);
    dockLayout->addWidget(m_jumpBackBtn);

    m_playPauseBtn = new QPushButton("> PLAY", m_dockWidget);
    m_playPauseBtn->setFont(BrutalistTheme::monospaceFont(9, QFont::Bold));
    m_playPauseBtn->setStyleSheet(BrutalistTheme::accentOrangeButtonStyleSheet());
    m_playPauseBtn->setToolTip("Toggle Play/Pause (Space)");
    m_playPauseBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_playPauseBtn, &QPushButton::clicked, this, &ViewfinderWidget::onPlayPauseClicked);
    dockLayout->addWidget(m_playPauseBtn);

    m_jumpFwdBtn = new QPushButton("+10s", m_dockWidget);
    m_jumpFwdBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_jumpFwdBtn->setToolTip("Jump Forward 10 Seconds (Right Arrow)");
    m_jumpFwdBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_jumpFwdBtn, &QPushButton::clicked, this, &ViewfinderWidget::onJumpForwardClicked);
    dockLayout->addWidget(m_jumpFwdBtn);

    m_stepFwdBtn = new QPushButton("1F >|", m_dockWidget);
    m_stepFwdBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_stepFwdBtn->setToolTip("Step Forward 1 Frame (.)");
    m_stepFwdBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_stepFwdBtn, &QPushButton::clicked, this, &ViewfinderWidget::onStepForwardClicked);
    dockLayout->addWidget(m_stepFwdBtn);

    m_stopBtn = new QPushButton("[] STOP", m_dockWidget);
    m_stopBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_stopBtn->setToolTip("Stop Playback");
    m_stopBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_stopBtn, &QPushButton::clicked, this, &ViewfinderWidget::onStopClicked);
    dockLayout->addWidget(m_stopBtn);

    m_shotBtn = new QPushButton("SHOT", m_dockWidget);
    m_shotBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_shotBtn->setToolTip("Take Forensic Screenshot (S / Shift+S)");
    m_shotBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_shotBtn, &QPushButton::clicked, this, [this]() {
        if (m_engine) m_engine->takeScreenshot();
    });
    dockLayout->addWidget(m_shotBtn);

    m_loopBtn = new QPushButton("A-B", m_dockWidget);
    m_loopBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_loopBtn->setToolTip("A-B Looper: Press [ for A, ] for B, \\ to clear");
    m_loopBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_loopBtn, &QPushButton::clicked, this, [this]() {
        if (!m_engine) return;
        if (!m_engine->isLoopActive()) {
            if (m_engine->loopPointA() < 0) {
                m_engine->setLoopPointA();
                m_loopBtn->setText("A:SET");
                m_loopBtn->setStyleSheet(BrutalistTheme::accentOrangeButtonStyleSheet());
            } else {
                m_engine->setLoopPointB();
                m_loopBtn->setText("A-B:ON");
                m_loopBtn->setStyleSheet(BrutalistTheme::accentLimeButtonStyleSheet());
            }
        } else {
            m_engine->clearLoop();
            m_loopBtn->setText("A-B");
            m_loopBtn->setStyleSheet("");
        }
    });
    dockLayout->addWidget(m_loopBtn);

    m_nightBtn = new QPushButton("NIGHT", m_dockWidget);
    m_nightBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_nightBtn->setToolTip("Toggle Night Mode Dynamic Dialogue Compressor (N)");
    m_nightBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_nightBtn, &QPushButton::clicked, this, [this]() {
        if (!m_engine) return;
        bool next = !m_engine->isNightMode();
        m_engine->setNightMode(next);
        m_nightBtn->setStyleSheet(next ? BrutalistTheme::accentLimeButtonStyleSheet() : "");
    });
    dockLayout->addWidget(m_nightBtn);

    dockLayout->addSpacing(10);

    // Speed selector
    auto *speedLabel = new QLabel("RATE:", m_dockWidget);
    speedLabel->setFont(BrutalistTheme::monospaceFont(8, QFont::Normal));
    speedLabel->setStyleSheet("color: #777788;");
    dockLayout->addWidget(speedLabel);

    m_speedCombo = new QComboBox(m_dockWidget);
    m_speedCombo->setFont(BrutalistTheme::monospaceFont(8, QFont::Normal));
    m_speedCombo->addItems({"0.5x", "0.75x", "1.0x", "1.25x", "1.5x", "1.75x", "2.0x"});
    m_speedCombo->setCurrentText("1.0x");
    m_speedCombo->setFocusPolicy(Qt::NoFocus);
    connect(m_speedCombo, &QComboBox::currentTextChanged, this, &ViewfinderWidget::onSpeedChanged);
    dockLayout->addWidget(m_speedCombo);

    dockLayout->addSpacing(10);

    // Audio stream selector
    auto *audLabel = new QLabel("AUD:", m_dockWidget);
    audLabel->setFont(BrutalistTheme::monospaceFont(8, QFont::Normal));
    audLabel->setStyleSheet("color: #777788;");
    dockLayout->addWidget(audLabel);

    m_audioTrackCombo = new QComboBox(m_dockWidget);
    m_audioTrackCombo->setFont(BrutalistTheme::monospaceFont(8, QFont::Normal));
    m_audioTrackCombo->addItem("Track 1 (Default)", 1);
    m_audioTrackCombo->setFocusPolicy(Qt::NoFocus);
    connect(m_audioTrackCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ViewfinderWidget::onAudioTrackChanged);
    dockLayout->addWidget(m_audioTrackCombo);

    // Subtitle stream selector
    auto *subLabel = new QLabel("SUB:", m_dockWidget);
    subLabel->setFont(BrutalistTheme::monospaceFont(8, QFont::Normal));
    subLabel->setStyleSheet("color: #777788;");
    dockLayout->addWidget(subLabel);

    m_subTrackCombo = new QComboBox(m_dockWidget);
    m_subTrackCombo->setFont(BrutalistTheme::monospaceFont(8, QFont::Normal));
    m_subTrackCombo->addItem("None", -1);
    m_subTrackCombo->setFocusPolicy(Qt::NoFocus);
    connect(m_subTrackCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ViewfinderWidget::onSubtitleTrackChanged);
    dockLayout->addWidget(m_subTrackCombo);

    dockLayout->addStretch(1);

    // Volume & Mute
    m_muteBtn = new QPushButton("VOL", m_dockWidget);
    m_muteBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_muteBtn->setFixedWidth(44);
    m_muteBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_muteBtn, &QPushButton::clicked, this, &ViewfinderWidget::onMuteClicked);
    dockLayout->addWidget(m_muteBtn);

    m_volumeSlider = new QSlider(Qt::Horizontal, m_dockWidget);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(85);
    m_volumeSlider->setFixedWidth(80);
    m_volumeSlider->setFocusPolicy(Qt::NoFocus);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &ViewfinderWidget::onVolumeSliderChanged);
    dockLayout->addWidget(m_volumeSlider);

    // Header buttons focus policy
    m_reticleToggleBtn->setFocusPolicy(Qt::NoFocus);
    m_osdToggleBtn->setFocusPolicy(Qt::NoFocus);
    m_fsToggleBtn->setFocusPolicy(Qt::NoFocus);
    m_switchModeBtn->setFocusPolicy(Qt::NoFocus);

    mainLayout->addWidget(m_dockWidget);

    m_videoSurface->installEventFilter(this);
    connect(m_videoSurface, &VideoSurfaceWidget::mouseMoved, this, &ViewfinderWidget::resetAutohideTimer);

    // Scrubber seek request connection
    connect(m_scrubber, &TickScrubberWidget::seekRequested, this, [this](qint64 ms) {
        if (m_engine) {
            m_engine->seek(ms);
        }
    });
}

void ViewfinderWidget::setPlaybackEngine(Core::PlaybackEngine *engine)
{
    m_engine = engine;
    if (m_videoSurface) {
        m_videoSurface->setPlaybackEngine(m_engine);
    }
    if (m_engine) {
        connectEngineSignals();
        updateUIFromEngine();
    }
}

void ViewfinderWidget::connectEngineSignals()
{
    if (!m_engine) return;

    connect(m_engine, &Core::PlaybackEngine::playbackStateChanged, this, &ViewfinderWidget::onEnginePlaybackStateChanged);
    connect(m_engine, &Core::PlaybackEngine::positionChanged, this, &ViewfinderWidget::onEnginePositionChanged);
    connect(m_engine, &Core::PlaybackEngine::durationChanged, this, &ViewfinderWidget::onEngineDurationChanged);
    connect(m_engine, &Core::PlaybackEngine::tracksChanged, this, &ViewfinderWidget::onEngineTracksChanged);
    connect(m_engine, &Core::PlaybackEngine::telemetryUpdated, this, &ViewfinderWidget::onEngineTelemetryUpdated);
    connect(m_engine, &Core::PlaybackEngine::metadataChanged, this, &ViewfinderWidget::onEngineMetadataChanged);
    connect(m_engine, &Core::PlaybackEngine::volumeChanged, this, [this](int vol) {
        m_volumeSlider->blockSignals(true);
        m_volumeSlider->setValue(vol);
        m_volumeSlider->blockSignals(false);
    });
    connect(m_engine, &Core::PlaybackEngine::muteChanged, this, [this](bool muted) {
        m_muteBtn->setText(muted ? "MUTE" : "VOL");
        m_muteBtn->setStyleSheet(muted ? "background-color: #FF4400; color: #070709;" : BrutalistTheme::primaryButtonStyleSheet());
    });
    connect(m_engine, &Core::PlaybackEngine::speedChanged, this, [this](double spd) {
        QString spdStr = QString("%1x").arg(spd, 0, 'f', (std::round(spd * 10) == spd * 10 ? 1 : 2));
        m_speedCombo->blockSignals(true);
        m_speedCombo->setCurrentText(spdStr);
        m_speedCombo->blockSignals(false);
    });
}

void ViewfinderWidget::updateUIFromEngine()
{
    if (!m_engine) return;
    onEnginePlaybackStateChanged(m_engine->playbackState());
    m_scrubber->setPositionMs(m_engine->positionMs());
    m_scrubber->setDurationMs(m_engine->durationMs());
    m_hud->updateDiagnostics(m_engine->telemetry());
    onEngineMetadataChanged(m_engine->metadata());
    onEngineTracksChanged(m_engine->audioTracks(), m_engine->subtitleTracks());
}

void ViewfinderWidget::toggleOsd()
{
    setOsdVisible(!m_osdVisible);
}

void ViewfinderWidget::toggleReticles()
{
    setReticlesVisible(!m_videoSurface->isReticlesVisible());
}

void ViewfinderWidget::setOsdVisible(bool visible)
{
    m_osdVisible = visible;
    m_hud->setVisible(visible);
    m_osdToggleBtn->setChecked(visible);
}

void ViewfinderWidget::setReticlesVisible(bool visible)
{
    m_videoSurface->setReticlesVisible(visible);
    m_reticleToggleBtn->setChecked(visible);
}

bool ViewfinderWidget::isOsdVisible() const
{
    return m_osdVisible;
}

bool ViewfinderWidget::isReticlesVisible() const
{
    return m_videoSurface->isReticlesVisible();
}

void ViewfinderWidget::onPlayPauseClicked()
{
    if (m_engine) {
        m_engine->togglePlayPause();
    }
}

void ViewfinderWidget::onStopClicked()
{
    if (m_engine) {
        m_engine->stop();
    }
}

void ViewfinderWidget::onStepBackwardClicked()
{
    if (m_engine) {
        m_engine->frameStep(-1);
    }
}

void ViewfinderWidget::onStepForwardClicked()
{
    if (m_engine) {
        m_engine->frameStep(1);
    }
}

void ViewfinderWidget::onJumpBackwardClicked()
{
    if (m_engine) {
        m_engine->seekRelative(-10000);
    }
}

void ViewfinderWidget::onJumpForwardClicked()
{
    if (m_engine) {
        m_engine->seekRelative(10000);
    }
}

void ViewfinderWidget::onSpeedChanged(const QString &speedStr)
{
    if (!m_engine) return;
    QString clean = speedStr;
    clean.remove('x');
    double val = clean.toDouble();
    if (val >= 0.25 && val <= 4.0) {
        m_engine->setSpeed(val);
    }
}

void ViewfinderWidget::onVolumeSliderChanged(int value)
{
    if (m_engine) {
        m_engine->setVolume(value);
    }
}

void ViewfinderWidget::onMuteClicked()
{
    if (m_engine) {
        m_engine->setMuted(!m_engine->isMuted());
    }
}

void ViewfinderWidget::onAudioTrackChanged(int index)
{
    if (m_blockTrackSignals || !m_engine) return;
    int trackId = m_audioTrackCombo->itemData(index).toInt();
    m_engine->setAudioTrack(trackId);
}

void ViewfinderWidget::onSubtitleTrackChanged(int index)
{
    if (m_blockTrackSignals || !m_engine) return;
    int trackId = m_subTrackCombo->itemData(index).toInt();
    m_engine->setSubtitleTrack(trackId);
}

void ViewfinderWidget::onEnginePlaybackStateChanged(Core::PlaybackState state)
{
    if (state == Core::PlaybackState::Playing) {
        m_playPauseBtn->setText("|| PAUSE");
        m_playPauseBtn->setStyleSheet(BrutalistTheme::accentOrangeButtonStyleSheet());
        resetAutohideTimer();
    } else {
        m_playPauseBtn->setText("> PLAY");
        m_playPauseBtn->setStyleSheet(BrutalistTheme::accentLimeButtonStyleSheet());
        m_autohideTimer.stop();
        setHeaderAndDocksVisible(true);
        setCursor(Qt::ArrowCursor);
        if (m_videoSurface) {
            m_videoSurface->setCursor(Qt::ArrowCursor);
        }
    }
}

bool ViewfinderWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseMove) {
        resetAutohideTimer();
    }
    return QWidget::eventFilter(watched, event);
}

void ViewfinderWidget::mouseMoveEvent(QMouseEvent *event)
{
    resetAutohideTimer();
    QWidget::mouseMoveEvent(event);
}

void ViewfinderWidget::wheelEvent(QWheelEvent *event)
{
    resetAutohideTimer();
    if (!m_engine) {
        QWidget::wheelEvent(event);
        return;
    }

    int numDegrees = event->angleDelta().y() / 8;
    int numSteps = numDegrees / 15;
    if (numSteps == 0 && numDegrees != 0) {
        numSteps = (numDegrees > 0) ? 1 : -1;
    }

    Qt::KeyboardModifiers mods = event->modifiers();
    if (mods & Qt::ControlModifier) {
        // Ctrl + Scroll = +/-10s seek
        qint64 seekMs = numSteps * 10000;
        m_engine->seekRelative(seekMs);
    } else if (mods & Qt::ShiftModifier) {
        // Shift + Scroll = single frame step
        m_engine->frameStep(numSteps > 0 ? 1 : -1);
    } else {
        // Vertical scroll = volume +/- 2%
        int curVol = m_engine->volume();
        int newVol = std::clamp(curVol + numSteps * 2, 0, 100);
        m_engine->setVolume(newVol);
    }

    event->accept();
}

void ViewfinderWidget::resetAutohideTimer()
{
    setHeaderAndDocksVisible(true);
    setCursor(Qt::ArrowCursor);
    if (m_videoSurface) {
        m_videoSurface->setCursor(Qt::ArrowCursor);
    }

    bool isFs = isFullScreen() || (window() && window()->isFullScreen());
    bool isPlaying = m_engine && m_engine->playbackState() == Core::PlaybackState::Playing;
    if (isFs && isPlaying) {
        m_autohideTimer.start(2500);
    } else {
        m_autohideTimer.stop();
    }
}

void ViewfinderWidget::onAutohideTimeout()
{
    bool isFs = isFullScreen() || (window() && window()->isFullScreen());
    bool isPlaying = m_engine && m_engine->playbackState() == Core::PlaybackState::Playing;
    if (isFs && isPlaying) {
        if ((m_dockWidget && m_dockWidget->underMouse()) ||
            (m_headerWidget && m_headerWidget->underMouse()) ||
            (m_scrubber && m_scrubber->underMouse())) {
            m_autohideTimer.start(2500);
            return;
        }
        setHeaderAndDocksVisible(false);
        setCursor(Qt::BlankCursor);
        if (m_videoSurface) {
            m_videoSurface->setCursor(Qt::BlankCursor);
        }
    }
}

void ViewfinderWidget::setHeaderAndDocksVisible(bool visible)
{
    if (m_headerWidget) m_headerWidget->setVisible(visible);
    if (m_dockWidget) m_dockWidget->setVisible(visible);
    if (m_scrubber) m_scrubber->setVisible(visible);
}

void ViewfinderWidget::onEnginePositionChanged(qint64 posMs, const QString & /*smpte*/)
{
    m_scrubber->setPositionMs(posMs);
}

void ViewfinderWidget::onEngineDurationChanged(qint64 durMs, const QString & /*smpte*/)
{
    m_scrubber->setDurationMs(durMs);
}

void ViewfinderWidget::onEngineTracksChanged(const QList<Core::TrackInfo> &audioTracks, const QList<Core::TrackInfo> &subTracks)
{
    m_blockTrackSignals = true;

    // Update audio track combo
    m_audioTrackCombo->clear();
    for (const auto &trk : audioTracks) {
        QString label = trk.title.isEmpty() ? QString("Track %1 (%2)").arg(trk.id).arg(trk.language.isEmpty() ? "und" : trk.language) : trk.title;
        m_audioTrackCombo->addItem(label, trk.id);
        if (trk.isSelected) {
            m_audioTrackCombo->setCurrentIndex(m_audioTrackCombo->count() - 1);
        }
    }
    if (m_audioTrackCombo->count() == 0) {
        m_audioTrackCombo->addItem("Default", 1);
    }

    // Update subtitle track combo
    m_subTrackCombo->clear();
    m_subTrackCombo->addItem("Disabled", -1);
    for (const auto &trk : subTracks) {
        QString label = trk.title.isEmpty() ? QString("Sub %1 (%2)").arg(trk.id).arg(trk.language.isEmpty() ? "und" : trk.language) : trk.title;
        m_subTrackCombo->addItem(label, trk.id);
        if (trk.isSelected) {
            m_subTrackCombo->setCurrentIndex(m_subTrackCombo->count() - 1);
        }
    }

    m_blockTrackSignals = false;
}

void ViewfinderWidget::onEngineTelemetryUpdated(const Core::DiagnosticsData &diag)
{
    m_hud->updateDiagnostics(diag);
}

void ViewfinderWidget::onEngineMetadataChanged(const Core::MediaMetadata &meta)
{
    QString title = meta.title.isEmpty() ? QFileInfo(meta.url).fileName() : meta.title;
    if (title.isEmpty()) title = "PENGUIN VIEWFINDER MONITOR";
    m_titleLabel->setText(title);
    m_videoSurface->setMediaTitle(title);
}

} // namespace UI
} // namespace Penguin
