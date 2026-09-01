#ifndef VUMETERWIDGET_H
#define VUMETERWIDGET_H

#include <QWidget>
#include <QColor>
#include "VUMeterDSP.h"

namespace Penguin {
namespace UI {

class VUMeterWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(double leftPeakDb READ leftPeakDb WRITE setLeftPeakDb)
    Q_PROPERTY(double rightPeakDb READ rightPeakDb WRITE setRightPeakDb)

public:
    explicit VUMeterWidget(QWidget *parent = nullptr);
    virtual ~VUMeterWidget() = default;

    double leftPeakDb() const { return m_levels.leftPeakDb; }
    double rightPeakDb() const { return m_levels.rightPeakDb; }
    double leftRmsDb() const { return m_levels.leftRmsDb; }
    double rightRmsDb() const { return m_levels.rightRmsDb; }
    bool isLeftClipped() const { return m_levels.leftClipped; }
    bool isRightClipped() const { return m_levels.rightClipped; }

public slots:
    void setLevels(double leftPeakDb, double rightPeakDb, double leftRmsDb = -60.0, double rightRmsDb = -60.0);
    void setVULevels(const Core::VULevels &levels);
    void setLeftPeakDb(double db);
    void setRightPeakDb(double db);
    void reset();

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void drawChannelBar(QPainter &p, const QRect &barRect, double peakDb, double peakHoldDb, bool clipped, const QString &label);
    double dbToX(double db, int width) const;

    Core::VULevels m_levels;
};

} // namespace UI
} // namespace Penguin

#endif // VUMETERWIDGET_H
