#ifndef CAMERAEXTENSION_H
#define CAMERAEXTENSION_H


#include <QCoreApplication>
#include "optodevkit/devkit.h"
#include "cameraviewwidget.h"
#include "cameraextensionform.h"

class CameraExtensionPlugin : public Plugin
{
	Q_OBJECT
		Q_PLUGIN_METADATA(IID Plugin_iid)
		Q_INTERFACES(Plugin)

public:
	CameraExtensionPlugin(QObject* parent = nullptr);
	~CameraExtensionPlugin();

	virtual InterfaceBase* getInterfaceInstance();

};

class CameraExtension : public Extension
{
	Q_OBJECT

	friend class CameraExtensionPlugin;
public:
	CameraExtension(QObject* parent = nullptr);
	~CameraExtension();

	virtual QWidget* getWidget() override;
	virtual void activateExtension() override;
	virtual void deactivateExtension() override;
	virtual void settingsLoaded(QVariantMap settings) override;

private:
	CameraExtensionForm* form;
	CameraViewWidget* cameraWidget;

public slots:
	void storeParameters();
	virtual void rawDataReceived(void* buffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame, unsigned int framesPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr) override;
	virtual void processedDataReceived(void* buffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame, unsigned int framesPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr) override;

signals:

};

#endif // CAMERAEXTENSION_H
