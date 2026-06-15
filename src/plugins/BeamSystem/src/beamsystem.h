

#ifndef OCTSYSTEMSIMULATORPLUGIN_H
#define OCTSYSTEMSIMULATORPLUGIN_H

#define STREAM_BUFFER_SIZE 2097152


#include "beamsystemsettingsdialog.h"
#include "optodevkit/devkit.h"
#include "optodevkit/qmmcore.h"
#include "optodevkit/CameraWorker.h"

#include <QObject>
#include <QCoreApplication>
#include <QThread>
#include <QDir>
#include <QDebug>
#include "math.h"
#include <fstream>

class BeamSystemPlugin : public Plugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID Plugin_iid)
	Q_INTERFACES(Plugin)

public:
	BeamSystemPlugin(QObject* parent = nullptr);
	~BeamSystemPlugin();

	virtual InterfaceBase* getInterfaceInstance();

};

class IdsCameraConfig;
class AcquisitionWorker;


class BeamSystem final : public AcquisitionSystem, public CameraWorker
{
	Q_OBJECT
	friend class BeamSystemPlugin;
public:
	explicit BeamSystem(QObject* parent = nullptr);
	~BeamSystem();

	QString name() const override;
	QString descr() const override;
	int width() const override;
	int height() const override;
	int bpp() const override;
	PixelScale sensorScale() const { return _pixelScale; }
	virtual QString formatBrightness(double v) const
	{
		if (_config.plot.normalize)
			return QString::number(v, 'f', 3);
		return QString::number(v, 'f', 0);
	}
	virtual PixelScale pixelScale() const {
		if (!_config.plot.rescale)
			return {};
		if (_config.plot.customScale.on)
			return _config.plot.customScale;
		return sensorScale();
	}

	virtual void startAcquisition() override;
	virtual void stopAcquisition() override;
	virtual void settingsLoaded(QVariantMap settings) override;

	virtual void logInfo(const QString msg) override;
	virtual void logError(const QString msg) override;

	bool canMeasure() const override { return true; }
	void startMeasure(MeasureSaver* saver) override;
	void stopMeasure() override;

	void initResultTableIntf(TableIntf* intf)override;
	void initPlotIntf(PlotIntf* plotIntf)override;

	HardConfigPanel* hardConfgPanel(QWidget* parent) override;

	virtual bool editConfig(int page = -1) override;

	virtual bool canMavg() const override{ return true; }

	virtual void setRoi(const RoiRect&) override;
	virtual void setRois(const QList<RoiRect>&)  override;
	virtual void setRois(const QList<QPointF>&)  override;
	virtual void setRoisSize(const FrameSize&)  override;
	virtual  void setRoiMode(RoiMode mode) override;
	virtual  RoiMode getRoiMode() override;
	virtual  RoiRect getRoi()override;
	virtual  QList<RoiRect> getRois() override;

	QList<QPair<int, QString>> measurCols() const override;

	IdsCameraConfig* idsConfig() { return _cfg.data(); }

private:
	bool _initSuccessfull;
	QImage _image;
	BeamSystemSettingsDialog* systemDialog;
	beamParams currParams;
	bool isCleanupPending ;
	QMMCore* _qmmCore;

	bool stopInterruption;
	bool init();
	void cleanup();
	void run();
	

	QString _name, _descr, _customId;
	int _width = 0;
	int _height = 0;
	PixelScale _pixelScale;

	QSharedPointer<IdsCameraConfig> _cfg;
	HardConfigPanel* _configPanel = nullptr;
	
	
	TableIntf* _tableIntf;
	PlotIntf* _plotIntf;

public slots:
	void slot_updateParams(beamParams newParams);
signals:
	void enableGui(bool enable);
};

#endif // 
