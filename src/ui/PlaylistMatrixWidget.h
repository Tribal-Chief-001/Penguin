#ifndef PLAYLISTMATRIXWIDGET_H
#define PLAYLISTMATRIXWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QList>
#include <QString>
#include <QPushButton>
#include <QLineEdit>

namespace Penguin {
namespace UI {

struct PlaylistItem {
    QString filePath;
    QString title;
    QString artist;
    QString album;
    qint64 durationMs = 0;
    QString format;
};

class PlaylistMatrixWidget : public QWidget {
    Q_OBJECT

public:
    explicit PlaylistMatrixWidget(QWidget *parent = nullptr);
    virtual ~PlaylistMatrixWidget() = default;

    int count() const { return m_items.size(); }
    int currentIndex() const { return m_currentIndex; }
    const QList<PlaylistItem>& items() const { return m_items; }
    PlaylistItem itemAt(int index) const;

public slots:
    void addItem(const QString &filePath, const QString &title = QString(),
                 const QString &artist = QString(), const QString &album = QString(),
                 qint64 durationMs = 0, const QString &format = QString());
    void addItems(const QList<PlaylistItem> &items);
    void setItems(const QList<PlaylistItem> &items, int currentIndex = -1);
    void removeItem(int index);
    void clearPlaylist();
    void setCurrentIndex(int index);
    void moveItem(int fromIndex, int toIndex);
    void shuffle();

signals:
    void trackDoubleClicked(int index, const PlaylistItem &item);
    void trackActivated(int index, const PlaylistItem &item);
    void currentIndexChanged(int index);
    void playlistChanged();
    void itemRemoved(int index);
    void itemMoved(int fromIndex, int toIndex);
    void playlistCleared();

private slots:
    void onCellDoubleClicked(int row, int column);
    void onFilterTextChanged(const QString &text);
    void onRemoveClicked();
    void onClearClicked();

private:
    void setupUI();
    void refreshTable();
    void updateRowHighlight();

    QList<PlaylistItem> m_items;
    int m_currentIndex = -1;

    QTableWidget *m_table = nullptr;
    QLineEdit *m_filterEdit = nullptr;
    QPushButton *m_removeBtn = nullptr;
    QPushButton *m_clearBtn = nullptr;
};

} // namespace UI
} // namespace Penguin

#endif // PLAYLISTMATRIXWIDGET_H
