#include "PlaylistMatrixWidget.h"
#include "BrutalistTheme.h"
#include "TimecodeFormatter.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileInfo>
#include <random>
#include <algorithm>

namespace Penguin {
namespace UI {

PlaylistMatrixWidget::PlaylistMatrixWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void PlaylistMatrixWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(4);

    // Top control bar: Filter input & Actions
    auto *topBar = new QHBoxLayout();
    topBar->setContentsMargins(4, 4, 4, 4);
    topBar->setSpacing(6);

    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText("SEARCH TRACK, ARTIST, ALBUM...");
    m_filterEdit->setFont(BrutalistTheme::monospaceFont(8, QFont::Normal));
    m_filterEdit->setStyleSheet(BrutalistTheme::searchInputStyleSheet());
    connect(m_filterEdit, &QLineEdit::textChanged, this, &PlaylistMatrixWidget::onFilterTextChanged);
    topBar->addWidget(m_filterEdit, 1);

    m_removeBtn = new QPushButton("REMOVE", this);
    m_removeBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_removeBtn->setStyleSheet(BrutalistTheme::pillButtonStyleSheet());
    connect(m_removeBtn, &QPushButton::clicked, this, &PlaylistMatrixWidget::onRemoveClicked);
    topBar->addWidget(m_removeBtn);

    m_clearBtn = new QPushButton("CLEAR", this);
    m_clearBtn->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
    m_clearBtn->setStyleSheet(BrutalistTheme::pillButtonStyleSheet());
    connect(m_clearBtn, &QPushButton::clicked, this, &PlaylistMatrixWidget::onClearClicked);
    topBar->addWidget(m_clearBtn);

    mainLayout->addLayout(topBar);

    // Matrix Table Widget
    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    QStringList headers = {"#", "TITLE", "ARTIST", "ALBUM", "DUR", "FORMAT"};
    m_table->setHorizontalHeaderLabels(headers);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setShowGrid(true);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setStyleSheet(BrutalistTheme::tableStyleSheet());
    m_table->setFont(BrutalistTheme::sansFont(9, QFont::Normal));

    connect(m_table, &QTableWidget::cellDoubleClicked, this, &PlaylistMatrixWidget::onCellDoubleClicked);

    mainLayout->addWidget(m_table, 1);
}

PlaylistItem PlaylistMatrixWidget::itemAt(int index) const
{
    if (index >= 0 && index < m_items.size()) {
        return m_items[index];
    }
    return PlaylistItem();
}

void PlaylistMatrixWidget::addItem(const QString &filePath, const QString &title,
                                  const QString &artist, const QString &album,
                                  qint64 durationMs, const QString &format)
{
    PlaylistItem item;
    item.filePath = filePath;
    item.title = title.isEmpty() ? QFileInfo(filePath).fileName() : title;
    item.artist = artist.isEmpty() ? "UNKNOWN ARTIST" : artist;
    item.album = album.isEmpty() ? "UNKNOWN ALBUM" : album;
    item.durationMs = durationMs;
    item.format = format.isEmpty() ? QFileInfo(filePath).suffix().toUpper() : format;

    m_items.append(item);
    refreshTable();
    emit playlistChanged();
}

void PlaylistMatrixWidget::addItems(const QList<PlaylistItem> &items)
{
    m_items.append(items);
    refreshTable();
    emit playlistChanged();
}

void PlaylistMatrixWidget::setItems(const QList<PlaylistItem> &items, int currentIndex)
{
    m_items = items;
    m_currentIndex = (currentIndex >= 0 && currentIndex < m_items.size()) ? currentIndex : (m_items.isEmpty() ? -1 : 0);
    refreshTable();
    emit playlistChanged();
}

void PlaylistMatrixWidget::removeItem(int index)
{
    if (index >= 0 && index < m_items.size()) {
        m_items.removeAt(index);
        if (m_currentIndex == index) {
            m_currentIndex = -1;
        } else if (m_currentIndex > index) {
            m_currentIndex--;
        }
        refreshTable();
        emit itemRemoved(index);
        emit playlistChanged();
    }
}

void PlaylistMatrixWidget::clearPlaylist()
{
    m_items.clear();
    m_currentIndex = -1;
    refreshTable();
    emit playlistCleared();
    emit playlistChanged();
}

void PlaylistMatrixWidget::setCurrentIndex(int index)
{
    if (index >= -1 && index < m_items.size() && index != m_currentIndex) {
        m_currentIndex = index;
        updateRowHighlight();
        emit currentIndexChanged(m_currentIndex);
    }
}

void PlaylistMatrixWidget::moveItem(int fromIndex, int toIndex)
{
    if (fromIndex >= 0 && fromIndex < m_items.size() &&
        toIndex >= 0 && toIndex < m_items.size() && fromIndex != toIndex) {
        PlaylistItem it = m_items.takeAt(fromIndex);
        m_items.insert(toIndex, it);
        if (m_currentIndex == fromIndex) {
            m_currentIndex = toIndex;
        } else if (fromIndex < m_currentIndex && toIndex >= m_currentIndex) {
            m_currentIndex--;
        } else if (fromIndex > m_currentIndex && toIndex <= m_currentIndex) {
            m_currentIndex++;
        }
        refreshTable();
        emit itemMoved(fromIndex, toIndex);
        emit playlistChanged();
    }
}

void PlaylistMatrixWidget::shuffle()
{
    if (m_items.size() <= 1) return;

    PlaylistItem currentItem;
    bool hasCurrent = (m_currentIndex >= 0 && m_currentIndex < m_items.size());
    if (hasCurrent) {
        currentItem = m_items[m_currentIndex];
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m_items.begin(), m_items.end(), g);

    if (hasCurrent) {
        for (int i = 0; i < m_items.size(); ++i) {
            if (m_items[i].filePath == currentItem.filePath) {
                m_currentIndex = i;
                break;
            }
        }
    }

    refreshTable();
    emit playlistChanged();
}

void PlaylistMatrixWidget::refreshTable()
{
    m_table->setRowCount(0);
    m_table->setRowCount(m_items.size());

    for (int r = 0; r < m_items.size(); ++r) {
        const auto &it = m_items[r];

        // 0: #
        QString numStr = QString("%1").arg(r + 1, 2, 10, QChar('0'));
        auto *numItem = new QTableWidgetItem(numStr);
        numItem->setTextAlignment(Qt::AlignCenter);
        numItem->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
        m_table->setItem(r, 0, numItem);

        // 1: TITLE
        auto *titleItem = new QTableWidgetItem(it.title);
        titleItem->setFont(BrutalistTheme::sansFont(9, QFont::Bold));
        m_table->setItem(r, 1, titleItem);

        // 2: ARTIST
        auto *artistItem = new QTableWidgetItem(it.artist);
        artistItem->setFont(BrutalistTheme::sansFont(8, QFont::Normal));
        artistItem->setForeground(BrutalistTheme::TEXT_SECONDARY_DIM);
        m_table->setItem(r, 2, artistItem);

        // 3: ALBUM
        auto *albumItem = new QTableWidgetItem(it.album);
        albumItem->setFont(BrutalistTheme::sansFont(8, QFont::Normal));
        albumItem->setForeground(BrutalistTheme::TEXT_SECONDARY_DIM);
        m_table->setItem(r, 3, albumItem);

        // 4: DURATION
        QString durStr = Core::TimecodeFormatter::formatSimpleTime(it.durationMs);
        auto *durItem = new QTableWidgetItem(durStr);
        durItem->setTextAlignment(Qt::AlignCenter);
        durItem->setFont(BrutalistTheme::monospaceFont(8, QFont::Normal));
        m_table->setItem(r, 4, durItem);

        // 5: FORMAT
        auto *fmtItem = new QTableWidgetItem(it.format);
        fmtItem->setTextAlignment(Qt::AlignCenter);
        fmtItem->setFont(BrutalistTheme::monospaceFont(8, QFont::Bold));
        fmtItem->setForeground(BrutalistTheme::ACCENT_TELEMETRY_CYAN);
        m_table->setItem(r, 5, fmtItem);
    }

    updateRowHighlight();
}

void PlaylistMatrixWidget::updateRowHighlight()
{
    for (int r = 0; r < m_table->rowCount(); ++r) {
        bool isActive = (r == m_currentIndex);
        for (int c = 0; c < m_table->columnCount(); ++c) {
            auto *cell = m_table->item(r, c);
            if (!cell) continue;

            if (isActive) {
                cell->setBackground(QColor(18, 32, 22, 220));
                if (c == 0) {
                    cell->setText(QString("ılı %1").arg(r + 1, 2, 10, QChar('0')));
                    cell->setForeground(BrutalistTheme::ACCENT_SIGNAL_LIME);
                } else if (c == 1) {
                    cell->setForeground(BrutalistTheme::ACCENT_SIGNAL_LIME);
                } else {
                    cell->setForeground(QColor("#DDFF88"));
                }
            } else {
                cell->setBackground(r % 2 == 0 ? QColor("#08080C") : QColor("#0B0B10"));
                if (c == 0) {
                    cell->setText(QString("%1").arg(r + 1, 2, 10, QChar('0')));
                    cell->setForeground(BrutalistTheme::TEXT_SECONDARY_DIM);
                } else if (c == 1) {
                    cell->setForeground(BrutalistTheme::TEXT_HIGH_CONTRAST);
                } else if (c == 5) {
                    cell->setForeground(BrutalistTheme::ACCENT_TELEMETRY_CYAN);
                } else {
                    cell->setForeground(BrutalistTheme::TEXT_SECONDARY_DIM);
                }
            }
        }
    }
}

void PlaylistMatrixWidget::onCellDoubleClicked(int row, int /*column*/)
{
    if (row >= 0 && row < m_items.size()) {
        m_currentIndex = row;
        updateRowHighlight();
        emit trackDoubleClicked(row, m_items[row]);
        emit trackActivated(row, m_items[row]);
    }
}

void PlaylistMatrixWidget::onFilterTextChanged(const QString &text)
{
    QString query = text.trimmed().toLower();
    for (int r = 0; r < m_items.size(); ++r) {
        if (query.isEmpty()) {
            m_table->setRowHidden(r, false);
        } else {
            const auto &it = m_items[r];
            bool match = it.title.toLower().contains(query) ||
                         it.artist.toLower().contains(query) ||
                         it.album.toLower().contains(query) ||
                         it.format.toLower().contains(query);
            m_table->setRowHidden(r, !match);
        }
    }
}

void PlaylistMatrixWidget::onRemoveClicked()
{
    int row = m_table->currentRow();
    if (row >= 0 && row < m_items.size()) {
        removeItem(row);
    }
}

void PlaylistMatrixWidget::onClearClicked()
{
    clearPlaylist();
}

} // namespace UI
} // namespace Penguin
