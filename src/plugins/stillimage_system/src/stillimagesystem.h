

#ifndef OCTSYSTEMSIMULATORPLUGIN_H
#define OCTSYSTEMSIMULATORPLUGIN_H

#define STREAM_BUFFER_SIZE 2097152

#include "stillimagesystemsettingsdialog.h"
#include "optodevkit/devkit.h"

#include <QObject>
#include <QCoreApplication>
#include <QThread>
#include <QDir>
#include <QDebug>
#include "math.h"
#include <fstream>

class StillImageSystemPlugin : public Plugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID Plugin_iid)
	Q_INTERFACES(Plugin)

public:
	StillImageSystemPlugin(QObject* parent = nullptr);
	~StillImageSystemPlugin();

	virtual InterfaceBase* getInterfaceInstance();

};

class StillImageSystem final : public AcquisitionSystem
{
	Q_OBJECT
	friend class StillImageSystemPlugin;
public:
	explicit StillImageSystem(QObject* parent = nullptr);
	~StillImageSystem();

	//QString name() const override;
	QString descr() const override;
	int width() const override;
	int height() const override;
	int bpp() const override;
	TableRowsSpec tableRows() const override;

	virtual void startAcquisition() override;
	virtual void stopAcquisition() override;
	virtual void settingsLoaded(QVariantMap settings) override;

	bool canMeasure() const override { return true; }
	void startMeasure(MeasureSaver* saver) override;
	void stopMeasure() override;

private:
	QImage _image;
	StillImageSystemSettingsDialog* systemDialog;
	imageParams currParams;
	bool isCleanupPending ;

	bool init();
	void cleanup();
	bool openFileToCopyToRam();
	void acqcuisitionSimulation();

	void measBeamBkgnd();

public slots:
	void slot_updateParams(imageParams newParams);

signals:
	void enableGui(bool enable);
};

#endif // 
