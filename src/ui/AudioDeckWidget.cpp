#include "AudioDeckWidget.h"
#include "BrutalistTheme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileInfo>

namespace Penguin {
namespace UI {

AudioDeckWidget::AudioDeckWidget(Core::PlaybackEngine *engine, QWidget *parent)
    : QWidget(parent)
    , m_engine(engine)
{
    setupUI();
    if (m_engine) {
        connectEngineSignals();
        updateUIFromEngine();
    }
}

void AudioDeckWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 1. Top Header Bar
    auto *headerWidget = new QWidget(this);
    headerWidget->setFixedHeight(32);
    headerWidget->setStyleSheet("background-color: #0B0B0E; border-bottom: 1px solid #1E1E24;");
    auto *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(8, 0, 8, 0);
    headerLayout->setSpacing(8);

    auto *modeTag = new QLabel("[MODE: HI-FI AUDIO DECK]", headerWidget);
    modeTag->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    modeTag->setStyleSheet("color: #CCFF00; border: none; background: transparent;");
    headerLayout->addWidget(modeTag);

    m_trackCounterLabel = new QLabel("TRACK 00 / 00", headerWidget);
    m_trackCounterLabel->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_trackCounterLabel->setStyleSheet("color: #777788; border: none; background: transparent;");
    headerLayout->addWidget(m_trackCounterLabel);

    headerLayout->addStretch(1);

    auto *switchModeBtn = new QPushButton("VIDEO VIEWFINDER [TAB]", headerWidget);
    switchModeBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    switchModeBtn->setStyleSheet(BrutalistTheme::accentOrangeButtonStyleSheet());
    connect(switchModeBtn, &QPushButton::clicked, this, &AudioDeckWidget::switchModeRequested);
    headerLayout->addWidget(switchModeBtn);

    mainLayout->addWidget(headerWidget);

    // 2. Main Content Splitter / Area
    auto *contentWidget = new QWidget(this);
    auto *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(8, 8, 8, 8);
    contentLayout->setSpacing(8);

    // --- Upper Half: Metadata Masthead (Left) + VU Meter & EQ Rack (Right) ---
    auto *upperContainer = new QWidget(contentWidget);
    auto *upperLayout = new QHBoxLayout(upperContainer);
    upperLayout->setContentsMargins(0, 0, 0, 0);
    upperLayout->setSpacing(8);

    // Metadata Masthead Card
    auto *mastheadCard = new QWidget(upperContainer);
    mastheadCard->setStyleSheet("background-color: #0B0B0E; border: 1px solid #1E1E24;");
    auto *mastheadLayout = new QHBoxLayout(mastheadCard);
    mastheadLayout->setContentsMargins(12, 12, 12, 12);
    mastheadLayout->setSpacing(12);

    // Album Artwork Frame
    m_coverArtBox = new QLabel(mastheadCard);
    m_coverArtBox->setFixedSize(110, 110);
    m_coverArtBox->setStyleSheet("background-color: #070709; border: 1px solid #333342; color: #444455;");
    m_coverArtBox->setAlignment(Qt::AlignCenter);
    m_coverArtBox->setFont(BrutalistTheme::monospaceFont(7, QFont::Bold));
    m_coverArtBox->setText("[ NO ART ]\n48kHz FLAC");
    mastheadLayout->addWidget(m_coverArtBox);

    // Metadata Text block
    auto *metaTextLayout = new QVBoxLayout();
    metaTextLayout->setSpacing(3);

    m_titleLabel = new QLabel("NO TRACK LOADED", mastheadCard);
    m_titleLabel->setFont(BrutalistTheme::sansFont(14, QFont::Bold));
    m_titleLabel->setStyleSheet("color: #FFFFFF; border: none; background: transparent;");
    m_titleLabel->setWordWrap(true);
    metaTextLayout->addWidget(m_titleLabel);

    m_artistLabel = new QLabel("UNKNOWN ARTIST", mastheadCard);
    m_artistLabel->setFont(BrutalistTheme::sansFont(10, QFont::Bold));
    m_artistLabel->setStyleSheet("color: #CCFF00; border: none; background: transparent;");
    metaTextLayout->addWidget(m_artistLabel);

    m_albumLabel = new QLabel("UNKNOWN ALBUM", mastheadCard);
    m_albumLabel->setFont(BrutalistTheme::sansFont(9, QFont::Normal));
    m_albumLabel->setStyleSheet("color: #777788; border: none; background: transparent;");
    metaTextLayout->addWidget(m_albumLabel);

    metaTextLayout->addSpacing(4);

    m_formatBadge = new QLabel("[AUDIO DECK // 24-BIT / 96.0 kHz / 2,450 kbps] [STEREO]", mastheadCard);
    m_formatBadge->setFont(BrutalistTheme::monospaceFont(7, QFont::Bold));
    m_formatBadge->setStyleSheet("color: #00E5FF; border: none; background: transparent;");
    metaTextLayout->addWidget(m_formatBadge);

    metaTextLayout->addStretch(1);
    mastheadLayout->addLayout(metaTextLayout, 1);

    upperLayout->addWidget(mastheadCard, 1);

    // Audio Analytics Rack (VU Meter + EQ)
    auto *rackContainer = new QWidget(upperContainer);
    auto *rackLayout = new QVBoxLayout(rackContainer);
    rackLayout->setContentsMargins(0, 0, 0, 0);
    rackLayout->setSpacing(6);

    m_vuMeter = new VUMeterWidget(rackContainer);
    m_vuMeter->setFixedHeight(68);
    rackLayout->addWidget(m_vuMeter);

    m_eqRack = new EqualizerRackWidget(rackContainer);
    rackLayout->addWidget(m_eqRack);

    upperLayout->addWidget(rackContainer, 1);
    contentLayout->addWidget(upperContainer);

    // --- Middle Half: Teleprompter (Left) + Playlist Matrix (Right) ---
    auto *middleContainer = new QWidget(contentWidget);
    auto *middleLayout = new QHBoxLayout(middleContainer);
    middleLayout->setContentsMargins(0, 0, 0, 0);
    middleLayout->setSpacing(8);

    m_teleprompter = new TeleprompterWidget(middleContainer);
    middleLayout->addWidget(m_teleprompter, 1);

    m_playlistMatrix = new PlaylistMatrixWidget(middleContainer);
    middleLayout->addWidget(m_playlistMatrix, 1);

    contentLayout->addWidget(middleContainer, 1);
    mainLayout->addWidget(contentWidget, 1);

    // 3. Mechanical Tick Scrubber
    m_scrubber = new TickScrubberWidget(this);
    mainLayout->addWidget(m_scrubber);

    // 4. Tactile Bottom Control Dock
    auto *dockWidget = new QWidget(this);
    dockWidget->setFixedHeight(44);
    dockWidget->setStyleSheet("background-color: #0B0B0E; border-top: 1px solid #1E1E24;");
    auto *dockLayout = new QHBoxLayout(dockWidget);
    dockLayout->setContentsMargins(8, 4, 8, 4);
    dockLayout->setSpacing(6);

    m_prevBtn = new QPushButton("|< PREV", dockWidget);
    m_prevBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_prevBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_prevBtn, &QPushButton::clicked, this, &AudioDeckWidget::previousTrack);
    dockLayout->addWidget(m_prevBtn);

    m_jumpBackBtn = new QPushButton("-10s", dockWidget);
    m_jumpBackBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_jumpBackBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_jumpBackBtn, &QPushButton::clicked, this, &AudioDeckWidget::onJumpBackwardClicked);
    dockLayout->addWidget(m_jumpBackBtn);

    m_playPauseBtn = new QPushButton("> PLAY", dockWidget);
    m_playPauseBtn->setFont(BrutalistTheme::monospaceFont(9, QFont::Bold));
    m_playPauseBtn->setStyleSheet(BrutalistTheme::accentLimeButtonStyleSheet());
    m_playPauseBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_playPauseBtn, &QPushButton::clicked, this, &AudioDeckWidget::onPlayPauseClicked);
    dockLayout->addWidget(m_playPauseBtn);

    m_jumpFwdBtn = new QPushButton("+10s", dockWidget);
    m_jumpFwdBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_jumpFwdBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_jumpFwdBtn, &QPushButton::clicked, this, &AudioDeckWidget::onJumpForwardClicked);
    dockLayout->addWidget(m_jumpFwdBtn);

    m_nextBtn = new QPushButton("NEXT >|", dockWidget);
    m_nextBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_nextBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_nextBtn, &QPushButton::clicked, this, &AudioDeckWidget::nextTrack);
    dockLayout->addWidget(m_nextBtn);

    dockLayout->addSpacing(12);

    m_shuffleBtn = new QPushButton("SHUFFLE: OFF", dockWidget);
    m_shuffleBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Normal));
    m_shuffleBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_shuffleBtn, &QPushButton::clicked, this, &AudioDeckWidget::toggleShuffle);
    dockLayout->addWidget(m_shuffleBtn);

    m_repeatBtn = new QPushButton("REPEAT: OFF", dockWidget);
    m_repeatBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Normal));
    m_repeatBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_repeatBtn, &QPushButton::clicked, this, &AudioDeckWidget::cycleRepeatMode);
    dockLayout->addWidget(m_repeatBtn);

    dockLayout->addStretch(1);

    m_muteBtn = new QPushButton("VOL", dockWidget);
    m_muteBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_muteBtn->setFixedWidth(44);
    m_muteBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_muteBtn, &QPushButton::clicked, this, &AudioDeckWidget::onMuteClicked);
    dockLayout->addWidget(m_muteBtn);

    m_volumeSlider = new QSlider(Qt::Horizontal, dockWidget);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(85);
    m_volumeSlider->setFixedWidth(80);
    m_volumeSlider->setFocusPolicy(Qt::NoFocus);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &AudioDeckWidget::onVolumeSliderChanged);
    dockLayout->addWidget(m_volumeSlider);

    m_scrubber->setFocusPolicy(Qt::NoFocus);
    switchModeBtn->setFocusPolicy(Qt::NoFocus);

    mainLayout->addWidget(dockWidget);

    // Internal Connections
    connect(m_scrubber, &TickScrubberWidget::seekRequested, this, [this](qint64 ms) {
        if (m_engine) m_engine->seek(ms);
    });

    connect(m_teleprompter, &TeleprompterWidget::seekRequested, this, [this](qint64 ms) {
        if (m_engine) m_engine->seek(ms);
    });

    connect(m_playlistMatrix, &PlaylistMatrixWidget::trackDoubleClicked, this, &AudioDeckWidget::onPlaylistTrackDoubleClicked);
    connect(m_playlistMatrix, &PlaylistMatrixWidget::playlistChanged, this, &AudioDeckWidget::updateTrackCounter);
    connect(m_playlistMatrix, &PlaylistMatrixWidget::currentIndexChanged, this, &AudioDeckWidget::updateTrackCounter);

    // EQ rack connections to engine
    connect(m_eqRack, &EqualizerRackWidget::bandGainChanged, this, [this](int band, double gain) {
        if (m_engine) m_engine->setEqualizerBand(band, gain);
    });
    connect(m_eqRack, &EqualizerRackWidget::presetChanged, this, [this](const QString &preset) {
        if (m_engine) m_engine->setEqualizerPreset(preset);
    });
}

void AudioDeckWidget::setPlaybackEngine(Core::PlaybackEngine *engine)
{
    m_engine = engine;
    if (m_engine) {
        connectEngineSignals();
        updateUIFromEngine();
    }
}

void AudioDeckWidget::connectEngineSignals()
{
    if (!m_engine) return;

    connect(m_engine, &Core::PlaybackEngine::playbackStateChanged, this, &AudioDeckWidget::onEnginePlaybackStateChanged);
    connect(m_engine, &Core::PlaybackEngine::positionChanged, this, &AudioDeckWidget::onEnginePositionChanged);
    connect(m_engine, &Core::PlaybackEngine::durationChanged, this, &AudioDeckWidget::onEngineDurationChanged);
    connect(m_engine, &Core::PlaybackEngine::metadataChanged, this, &AudioDeckWidget::onEngineMetadataChanged);
    connect(m_engine, &Core::PlaybackEngine::vuLevelsChanged, this, &AudioDeckWidget::onEngineVuLevelsChanged);
    connect(m_engine, &Core::PlaybackEngine::activeLyricChanged, this, &AudioDeckWidget::onEngineActiveLyricChanged);
    connect(m_engine, &Core::PlaybackEngine::mediaFinished, this, &AudioDeckWidget::onEngineMediaFinished);

    connect(m_engine, &Core::PlaybackEngine::volumeChanged, this, [this](int vol) {
        m_volumeSlider->blockSignals(true);
        m_volumeSlider->setValue(vol);
        m_volumeSlider->blockSignals(false);
    });
    connect(m_engine, &Core::PlaybackEngine::muteChanged, this, [this](bool muted) {
        m_muteBtn->setText(muted ? "MUTE" : "VOL");
        m_muteBtn->setStyleSheet(muted ? "background-color: #FF4400; color: #070709;" : BrutalistTheme::primaryButtonStyleSheet());
    });
}

void AudioDeckWidget::updateUIFromEngine()
{
    if (!m_engine) return;
    onEnginePlaybackStateChanged(m_engine->playbackState());
    m_scrubber->setPositionMs(m_engine->positionMs());
    m_scrubber->setDurationMs(m_engine->durationMs());
    onEngineMetadataChanged(m_engine->metadata());
    m_teleprompter->setLrcParser(m_engine->lrcParser());
    m_eqRack->setAllGains(m_engine->equalizer().allGains());
    updateTrackCounter();
}

void AudioDeckWidget::updateTrackCounter()
{
    int total = m_playlistMatrix->count();
    int current = m_playlistMatrix->currentIndex() + 1;
    m_trackCounterLabel->setText(QString("TRACK %1 / %2")
        .arg(current, 2, 10, QChar('0'))
        .arg(total, 2, 10, QChar('0')));
}

void AudioDeckWidget::playTrackAtIndex(int index)
{
    if (index < 0 || index >= m_playlistMatrix->count()) return;

    m_playlistMatrix->setCurrentIndex(index);
    PlaylistItem item = m_playlistMatrix->itemAt(index);
    if (!item.filePath.isEmpty() && m_engine) {
        m_engine->loadMedia(item.filePath, true);
    }
}

void AudioDeckWidget::nextTrack()
{
    if (m_playlistMatrix->count() == 0) return;

    int nextIdx = m_playlistMatrix->currentIndex() + 1;
    if (nextIdx >= m_playlistMatrix->count()) {
        if (m_repeatMode == RepeatMode::RepeatAll) {
            nextIdx = 0;
        } else {
            return;
        }
    }
    playTrackAtIndex(nextIdx);
}

void AudioDeckWidget::previousTrack()
{
    if (m_playlistMatrix->count() == 0) return;

    if (m_engine && m_engine->positionMs() > 3000) {
        // If > 3 seconds into track, restart current track
        m_engine->seek(0);
        return;
    }

    int prevIdx = m_playlistMatrix->currentIndex() - 1;
    if (prevIdx < 0) {
        if (m_repeatMode == RepeatMode::RepeatAll) {
            prevIdx = m_playlistMatrix->count() - 1;
        } else {
            prevIdx = 0;
        }
    }
    playTrackAtIndex(prevIdx);
}

void AudioDeckWidget::toggleShuffle()
{
    m_shuffleEnabled = !m_shuffleEnabled;
    m_shuffleBtn->setText(m_shuffleEnabled ? "SHUFFLE: ON" : "SHUFFLE: OFF");
    m_shuffleBtn->setStyleSheet(m_shuffleEnabled ? "background-color: #1E1E24; color: #CCFF00; border: 1px solid #CCFF00;" : BrutalistTheme::primaryButtonStyleSheet());

    if (m_shuffleEnabled) {
        m_playlistMatrix->shuffle();
    }
}

void AudioDeckWidget::cycleRepeatMode()
{
    if (m_repeatMode == RepeatMode::Off) {
        m_repeatMode = RepeatMode::RepeatAll;
        m_repeatBtn->setText("REPEAT: ALL");
        m_repeatBtn->setStyleSheet("background-color: #1E1E24; color: #CCFF00; border: 1px solid #CCFF00;");
    } else if (m_repeatMode == RepeatMode::RepeatAll) {
        m_repeatMode = RepeatMode::RepeatOne;
        m_repeatBtn->setText("REPEAT: ONE");
        m_repeatBtn->setStyleSheet("background-color: #1E1E24; color: #FF4400; border: 1px solid #FF4400;");
    } else {
        m_repeatMode = RepeatMode::Off;
        m_repeatBtn->setText("REPEAT: OFF");
        m_repeatBtn->setStyleSheet(BrutalistTheme::primaryButtonStyleSheet());
    }
}

void AudioDeckWidget::onPlayPauseClicked()
{
    if (m_engine) {
        m_engine->togglePlayPause();
    }
}

void AudioDeckWidget::onJumpBackwardClicked()
{
    if (m_engine) {
        m_engine->seekRelative(-10000);
    }
}

void AudioDeckWidget::onJumpForwardClicked()
{
    if (m_engine) {
        m_engine->seekRelative(10000);
    }
}

void AudioDeckWidget::onVolumeSliderChanged(int value)
{
    if (m_engine) {
        m_engine->setVolume(value);
    }
}

void AudioDeckWidget::onMuteClicked()
{
    if (m_engine) {
        m_engine->setMuted(!m_engine->isMuted());
    }
}

void AudioDeckWidget::onPlaylistTrackDoubleClicked(int index, const PlaylistItem &item)
{
    m_playlistMatrix->setCurrentIndex(index);
    if (!item.filePath.isEmpty() && m_engine) {
        m_engine->loadMedia(item.filePath, true);
    }
}

void AudioDeckWidget::onEnginePlaybackStateChanged(Core::PlaybackState state)
{
    if (state == Core::PlaybackState::Playing) {
        m_playPauseBtn->setText("|| PAUSE");
        m_playPauseBtn->setStyleSheet(BrutalistTheme::accentOrangeButtonStyleSheet());
    } else {
        m_playPauseBtn->setText("> PLAY");
        m_playPauseBtn->setStyleSheet(BrutalistTheme::accentLimeButtonStyleSheet());
    }
}

void AudioDeckWidget::onEnginePositionChanged(qint64 posMs, const QString & /*smpte*/)
{
    m_scrubber->setPositionMs(posMs);
    m_teleprompter->setPositionMs(posMs);
}

void AudioDeckWidget::onEngineDurationChanged(qint64 durMs, const QString & /*smpte*/)
{
    m_scrubber->setDurationMs(durMs);
}

void AudioDeckWidget::onEngineMetadataChanged(const Core::MediaMetadata &meta)
{
    QString title = meta.title.isEmpty() ? QFileInfo(meta.url).fileName() : meta.title;
    if (title.isEmpty()) title = "NO TRACK LOADED";

    m_titleLabel->setText(title);
    m_artistLabel->setText(meta.artist.isEmpty() ? "UNKNOWN ARTIST" : meta.artist);
    m_albumLabel->setText(meta.album.isEmpty() ? "UNKNOWN ALBUM" : meta.album);

    QString codec = meta.format.isEmpty() ? (m_engine ? m_engine->telemetry().audioCodec.toUpper() : "FLAC") : meta.format.toUpper();
    if (codec.isEmpty()) codec = "FLAC";
    int sRate = (m_engine && m_engine->telemetry().audioSampleRate > 0) ? m_engine->telemetry().audioSampleRate : 48000;
    qint64 kbps = (m_engine && m_engine->telemetry().audioBitrate > 0) ? m_engine->telemetry().audioBitrate / 1000 : 1411;
    m_formatBadge->setText(QString("[%1 // %2.1f kHz // %3 kbps] [STEREO]")
        .arg(codec)
        .arg(sRate / 1000.0, 0, 'f', 1)
        .arg(kbps));

    m_teleprompter->setLrcParser(m_engine ? m_engine->lrcParser() : Core::LrcParser());
    updateTrackCounter();
}

void AudioDeckWidget::onEngineVuLevelsChanged(double lPeak, double rPeak, double lRms, double rRms)
{
    m_vuMeter->setLevels(lPeak, rPeak, lRms, rRms);
}

void AudioDeckWidget::onEngineActiveLyricChanged(int cueIndex, const QString & /*lyricText*/)
{
    m_teleprompter->setActiveCueIndex(cueIndex);
}

void AudioDeckWidget::onEngineMediaFinished()
{
    if (m_repeatMode == RepeatMode::RepeatOne) {
        if (m_engine) {
            m_engine->seek(0);
            m_engine->play();
        }
    } else {
        nextTrack();
    }
}

} // namespace UI
} // namespace Penguin
