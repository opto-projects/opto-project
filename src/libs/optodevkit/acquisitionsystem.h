
#ifndef ACQUISITIONSYSTEM_H
#define ACQUISITIONSYSTEM_H

#include "plugin.h"

#include "TableIntf.h"
#include "PlotIntf.h"
#include "CameraTypes.h"
#include "HardConfigPanel.h"

#include <QObject>
#include <QEventLoop>
#include <QDialog>
#include <QDebug>
#include <QThread>
#include <QString>
#include <QMutex>

class MeasureSaver;
class CameraWorker;
class PlotIntf;
class TableIntf;

class QWidget;
class ConfigDlgOpts;

class AcquisitionSystem : public InterfaceBase
{
	Q_OBJECT
public:
	AcquisitionSystem(QObject* parent = nullptr);

    virtual ~AcquisitionSystem();

    virtual QString name() const { return {}; }
    virtual QString descr() const { return {}; }
    virtual int width() const = 0;
    virtual int height() const = 0;
    virtual int bpp() const = 0;
    virtual PixelScale sensorScale() const { return {}; }
    QString resolutionStr() const
    {
        return QStringLiteral("%1 ¡Á %2 ¡Á %3bit").arg(width()).arg(height()).arg(bpp());
    }
    virtual QString formatBrightness(double v) const
    {
        return QString::number(v, 'f', 0);
    } 
    virtual PixelScale pixelScale() const {
        return sensorScale();
    }

    virtual void startAcquisition() = 0;
    virtual bool isAcquisition() const { return acqusitionRunning; }
    virtual void stopAcquisition() {};

    virtual bool canMeasure() const { return false; }
    virtual void startMeasure(MeasureSaver* ) {}
    virtual void stopMeasure() {}

    virtual void initResultTableIntf(TableIntf* intf) {}
    virtual void initPlotIntf(PlotIntf* plotIntf) {}

    virtual void saveHardConfig(QSettings*) {}
    virtual HardConfigPanel* hardConfgPanel(QWidget* parent) { return nullptr; }

    virtual QList<QPair<int, QString>> measurCols() const { return {}; }
    virtual bool editConfig(int page = -1) { return false; }

    virtual bool canMavg() const { return false; }

    virtual void setRoi(const RoiRect&) {}
    virtual void setRois(const QList<RoiRect>&) {}
    virtual void setRois(const QList<QPointF>&) {}
    virtual void setRoisSize(const FrameSize&) {}
    virtual  void setRoiMode(RoiMode mode) {}

    virtual  RoiMode getRoiMode() { return {}; }
    virtual  RoiRect getRoi() { return {}; }
    virtual  QList<RoiRect> getRois() { return {}; }


    //write to log file if AcquisitionSystem support
    virtual void logInfo(const QString msg) {};
    virtual void logError(const QString msg) {};

    //virtual IdsCameraConfig* idsConfig() { return nullptr; }

    QWidget* settingsForm; ///< Widget that is displayed to the user

    QVariantMap settingsMap;

protected:

    bool acqusitionRunning;
    QMutex acqusitionMutex;
    QMutex stopInterruptionMutex;
    


    virtual void initConfigMore(ConfigDlgOpts& opts) {}
    virtual void loadConfigMore(QSettings*) {}
    virtual void saveConfigMore(QSettings*) {}

signals:
    void newImageReady(void* frame, unsigned int bitDepth, unsigned int width, unsigned int height, unsigned int channel);
	void acquisitionStarted(AcquisitionSystem*); 
	void acquisitionStopped();
    void status(const CameraStats& stats);
    void ready();
	
};


#endif // ACQUISITIONSYSTEM_H

