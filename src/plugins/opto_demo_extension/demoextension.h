

#ifndef DEMOEXTENSION_H
#define DEMOEXTENSION_H

#include "optodevkit/devkit.h"
#include "demoextensionform.h"
#include <QCoreApplication>

class DemoPlugin : public Plugin
{
	Q_OBJECT
		Q_PLUGIN_METADATA(IID Plugin_iid)
		Q_INTERFACES(Plugin)

public:
	DemoPlugin(QObject* parent = nullptr);
	~DemoPlugin();

	virtual InterfaceBase* getInterfaceInstance();

};


class DemoExtension : public Extension
{
	Q_OBJECT

	friend class DemoPlugin;
public:
	DemoExtension(QObject* parent = nullptr);
	~DemoExtension();

	virtual QWidget* getWidget() override;
	virtual void activateExtension() override;
	virtual void deactivateExtension() override;
	virtual void settingsLoaded(QVariantMap settings) override;


private:
	DemoExtensionForm* form;
	demoParams currentParameters;
	bool widgetDisplayed;
	bool isCalculating;
	bool active;

public slots:
	void setParameters(demoParams params);
	virtual void rawDataReceived(void* buffer, unsigned bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame, unsigned int framesPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr) override;
	virtual void processedDataReceived(void* buffer, unsigned bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame, unsigned int framesPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr) override;

signals:

};

#endif // DEMOEXTENSION_H
