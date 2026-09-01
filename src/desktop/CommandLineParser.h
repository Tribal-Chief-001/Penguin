#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QString>
#include <QStringList>
#include <QCommandLineParser>

namespace Penguin {
namespace Desktop {

struct CommandLineOptions {
    QStringList files;
    bool forceAudio = false;
    bool forceVideo = false;
    bool fullscreen = false;
    int volume = -1; // -1 if not specified, 0..100
    double speed = -1.0; // -1.0 if not specified, 0.5..2.0
    QString subtitlePath;
    QString loopMode = "none"; // "none", "track", "playlist"
    bool shuffle = false;
    QString eqPreset;

    // Remote Actions
    QString ipcAction; // "play-pause", "next", "previous", "stop", "raise", or empty

    // Diagnostic & Self-Test
    bool testMode = false;
    bool devTelemetry = false;

    // Standard Info
    bool helpRequested = false;
    bool versionRequested = false;

    bool isIpcAction() const { return !ipcAction.isEmpty(); }
};

class CommandLineParser {
public:
    CommandLineParser();

    CommandLineOptions parse(const QStringList &arguments, QString *errorMessage = nullptr);
    QString helpText() const;
    QString versionText() const;

private:
    void setupParser();

    QCommandLineParser m_parser;
};

} // namespace Desktop
} // namespace Penguin

#endif // COMMANDLINEPARSER_H
