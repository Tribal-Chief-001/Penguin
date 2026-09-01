#ifndef DIAGNOSTICSHUDWIDGET_H
#define DIAGNOSTICSHUDWIDGET_H

#include <QWidget>
#include <QString>
#include "MpvBackend.h"

namespace Penguin {
namespace UI {

class DiagnosticsHUDWidget : public QWidget {
    Q_OBJECT

public:
    explicit DiagnosticsHUDWidget(QWidget *parent = nullptr);
    virtual ~DiagnosticsHUDWidget() = default;

    const Core::DiagnosticsData& diagnostics() const { return m_diag; }

public slots:
    void updateDiagnostics(const Core::DiagnosticsData &diag);
    void setFps(double actualFps, double nominalFps = 0.0);
    void setDroppedFrames(qint64 dropped);
    void setVideoCodec(const QString &codec);
    void setAudioCodec(const QString &codec);
    void setBitrate(qint64 bps);
    void setResolution(int w, int h);
    void setRenderTimeMs(double ms);
    void setAvSkewMs(double ms);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    Core::DiagnosticsData m_diag;
};

} // namespace UI
} // namespace Penguin

#endif // DIAGNOSTICSHUDWIDGET_H
