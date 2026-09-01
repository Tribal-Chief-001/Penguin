#include "CommandLineParser.h"

#include <QFileInfo>

namespace Penguin {
namespace Desktop {

CommandLineParser::CommandLineParser()
{
    setupParser();
}

void CommandLineParser::setupParser()
{
    m_parser.setApplicationDescription("Tactile Digital Brutalist Audio & Video Player for Linux.");
    m_parser.addHelpOption();

    QCommandLineOption versionOption(QStringList() << "V" << "version", "Displays version information.");
    m_parser.addOption(versionOption);

    // Mode options
    QCommandLineOption audioOption(QStringList() << "a" << "audio", "Force Hi-Fi Audio Deck UI Mode.");
    m_parser.addOption(audioOption);

    QCommandLineOption videoOption(QStringList() << "v" << "video", "Force Viewfinder Video UI Mode.");
    m_parser.addOption(videoOption);

    // Window options
    QCommandLineOption fullscreenOption(QStringList() << "f" << "fullscreen", "Launch in fullscreen mode.");
    m_parser.addOption(fullscreenOption);

    // Audio & Rate controls
    QCommandLineOption volumeOption(QStringList() << "volume", "Set initial audio volume (0-100).", "VOLUME");
    m_parser.addOption(volumeOption);

    QCommandLineOption speedOption(QStringList() << "speed", "Set playback speed multiplier (0.5 to 2.0).", "SPEED");
    m_parser.addOption(speedOption);

    QCommandLineOption subOption(QStringList() << "sub", "Attach external subtitle file (.srt/.ass/.vtt).", "SUBTITLE_FILE");
    m_parser.addOption(subOption);

    QCommandLineOption loopOption(QStringList() << "loop", "Set playlist loop mode (none, track, playlist).", "LOOP_MODE", "none");
    m_parser.addOption(loopOption);

    QCommandLineOption shuffleOption(QStringList() << "shuffle", "Enable random shuffle mode.");
    m_parser.addOption(shuffleOption);

    QCommandLineOption eqOption(QStringList() << "eq", "Apply equalizer preset by name.", "PRESET");
    m_parser.addOption(eqOption);

    // IPC Action options
    QCommandLineOption actionOption(QStringList() << "action", "Send IPC control action (play-pause, next, previous, stop, raise).", "ACTION");
    m_parser.addOption(actionOption);

    QCommandLineOption togglePauseOption(QStringList() << "toggle-pause", "Send play/pause toggle command to running instance.");
    m_parser.addOption(togglePauseOption);

    QCommandLineOption nextOption(QStringList() << "next", "Send next track command to running instance.");
    m_parser.addOption(nextOption);

    QCommandLineOption prevOption(QStringList() << "prev" << "previous", "Send previous track command to running instance.");
    m_parser.addOption(prevOption);

    QCommandLineOption stopOption(QStringList() << "stop", "Send stop command to running instance.");
    m_parser.addOption(stopOption);

    // Self-tests & diagnostics
    QCommandLineOption testOption(QStringList() << "test" << "headless-test", "Run automated test suite headlessly.");
    m_parser.addOption(testOption);

    QCommandLineOption devTelemetryOption(QStringList() << "dev-telemetry", "Enable OSD diagnostic HUD overlay.");
    m_parser.addOption(devTelemetryOption);

    // Positional media files
    m_parser.addPositionalArgument("files", "Media file paths, directories, or stream URLs to open and play.", "[files...]");
}

CommandLineOptions CommandLineParser::parse(const QStringList &arguments, QString *errorMessage)
{
    CommandLineOptions options;

    if (!m_parser.parse(arguments)) {
        if (errorMessage) {
            *errorMessage = m_parser.errorText();
        }
        return options;
    }

    if (m_parser.isSet("help")) {
        options.helpRequested = true;
        return options;
    }
    if (m_parser.isSet("version")) {
        options.versionRequested = true;
        return options;
    }

    options.forceAudio = m_parser.isSet("audio");
    options.forceVideo = m_parser.isSet("video");
    options.fullscreen = m_parser.isSet("fullscreen");

    if (m_parser.isSet("volume")) {
        bool ok = false;
        int vol = m_parser.value("volume").toInt(&ok);
        if (ok && vol >= 0 && vol <= 100) {
            options.volume = vol;
        } else if (errorMessage) {
            *errorMessage = "Volume must be an integer between 0 and 100.";
        }
    }

    if (m_parser.isSet("speed")) {
        bool ok = false;
        double sp = m_parser.value("speed").toDouble(&ok);
        if (ok && sp >= 0.25 && sp <= 4.0) {
            options.speed = sp;
        }
    }

    if (m_parser.isSet("sub")) {
        options.subtitlePath = m_parser.value("sub");
    }

    if (m_parser.isSet("loop")) {
        QString l = m_parser.value("loop").toLower();
        if (l == "track" || l == "playlist" || l == "none") {
            options.loopMode = l;
        }
    }

    options.shuffle = m_parser.isSet("shuffle");

    if (m_parser.isSet("eq")) {
        options.eqPreset = m_parser.value("eq");
    }

    // IPC Action resolution
    if (m_parser.isSet("action")) {
        options.ipcAction = m_parser.value("action").toLower();
    } else if (m_parser.isSet("toggle-pause")) {
        options.ipcAction = "play-pause";
    } else if (m_parser.isSet("next")) {
        options.ipcAction = "next";
    } else if (m_parser.isSet("prev")) {
        options.ipcAction = "previous";
    } else if (m_parser.isSet("stop")) {
        options.ipcAction = "stop";
    }

    options.testMode = m_parser.isSet("test");
    options.devTelemetry = m_parser.isSet("dev-telemetry");

    options.files = m_parser.positionalArguments();

    return options;
}

QString CommandLineParser::helpText() const
{
    return m_parser.helpText();
}

QString CommandLineParser::versionText() const
{
    return "Penguin 1.0.0 (Tactile Digital Brutalism // Studio Precision)";
}

} // namespace Desktop
} // namespace Penguin
