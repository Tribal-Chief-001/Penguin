#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <memory>

#include "PlaybackEngine.h"
#include "ViewfinderWidget.h"
#include "AudioDeckWidget.h"

namespace Penguin {
namespace Library {
class StatePersistence;
class PlaylistManager;
}

namespace UI {

enum class UIMode {
    VideoViewfinder = 0,
    HiFiAudioDeck = 1
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow() = default;

    UIMode currentMode() const { return m_currentMode; }
    Core::PlaybackEngine* playbackEngine() const { return m_engine.get(); }
    ViewfinderWidget* viewfinderWidget() const { return m_viewfinder; }
    AudioDeckWidget* audioDeckWidget() const { return m_audioDeck; }

    void setStatePersistence(Library::StatePersistence *persistence);
    Library::StatePersistence* statePersistence() const { return m_persistence; }

    void setPlaylistManager(Library::PlaylistManager *playlistMgr);
    Library::PlaylistManager* playlistManager() const { return m_playlistMgr; }

    void saveState();
    void restoreState();

public slots:
    void setMode(UIMode mode);
    void toggleMode();
    bool openMedia(const QString &filePath, bool autoPlay = true);
    void openDirectory(const QString &dirPath);
    void openFileDialog();
    void openDirectoryDialog();
    void openUrlDialog();
    void toggleFullscreen();
    void takeScreenshot(bool includeSubtitles = false);
    void setLoopPointA();
    void setLoopPointB();
    void clearLoop();
    void adjustSubtitleDelay(int deltaMs);
    void adjustAudioDelay(int deltaMs);
    void toggleNightMode();
    void toggleCrossfeed();
    void toggleDeband();
    void cycleAspectRatio();
    void togglePipMode();
    void cycleSecondarySubtitle();
    void addBookmark();
    void nextBookmark();
    void adjustPitch(double deltaSemitones);
    void nextChapter();
    void previousChapter();
    void openVideoEqualizerDialog();
    void showOsdMessage(const QString &message, int durationMs = 2500);

protected:
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onEngineMetadataChanged(const Core::MediaMetadata &meta);
    void onEngineErrorOccurred(const QString &errorMessage);
    void onEngineMediaFinished();

private:
    void setupUI();
    void setupShortcuts();
    void setupConnections();

    std::unique_ptr<Core::PlaybackEngine> m_engine;
    QStackedWidget *m_stackedWidget = nullptr;
    ViewfinderWidget *m_viewfinder = nullptr;
    AudioDeckWidget *m_audioDeck = nullptr;

    Library::StatePersistence *m_persistence = nullptr;
    Library::PlaylistManager *m_playlistMgr = nullptr;

    UIMode m_currentMode = UIMode::VideoViewfinder;
    bool m_isPip = false;
    QRect m_savedGeometry;
};

} // namespace UI
} // namespace Penguin

#endif // MAINWINDOW_H
