

#ifndef DEMOEXTENSION_H
#define DEMOEXTENSION_H

#define NUMBER_OF_BUFFERS 2

#include <QCoreApplication>
#include <QThread>
#include "optodevkit/devkit.h"
#include "imagestatisticsextensionform.h"
#include "imagestatisticscalculator.h"
#include "roiselector.h"

class ImageStatisticsPlugin : public Plugin
{
	Q_OBJECT
		Q_PLUGIN_METADATA(IID Plugin_iid)
		Q_INTERFACES(Plugin)

public:
	ImageStatisticsPlugin(QObject* parent = nullptr);
	~ImageStatisticsPlugin();

	virtual InterfaceBase* getInterfaceInstance();

};


class ImageStatisticsExtension : public Extension
{
	Q_OBJECT

	QThread statisticsCalculatorThread;
	friend class ImageStatisticsPlugin;
public:
	ImageStatisticsExtension(QObject* parent = nullptr);
	~ImageStatisticsExtension();

	virtual QWidget* getWidget() override;
	virtual void activateExtension() override;
	virtual void deactivateExtension() override;
	virtual void settingsLoaded(QVariantMap settings) override;


private:
	ImageStatisticsCalculator* statisticsCalculator;
	ROISelector* roiSelect;
	QVector<void*> frameBuffersRaw;
	QVector<void*> frameBuffersProcessed;
	int copyBufferId;
	size_t bytesPerFrameRaw;
	size_t bytesPerFrameProcessed;

	ImageStatisticsExtensionForm* form;
	bool widgetDisplayed;
	bool isCalculating;
	bool active;

	int lostBuffersRaw;
	int lostBuffersProcessed;
	BUFFER_SOURCE bufferSource;
	int frameNr;
	int bufferNr;
	unsigned int framesPerBuffer;
	unsigned int buffersPerVolume;

	void releaseFrameBuffers(QVector<void*> buffers);

public slots:
	void storeParameters();
	void setBufferSource(BUFFER_SOURCE src){this->bufferSource = src;}
	void setFrameNr(int frameNr);
	void setBufferNr(int bufferNr);
	virtual void rawDataReceived(void* buffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame, unsigned int framesPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr) override;
	virtual void processedDataReceived(void* buffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame, unsigned int framesPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr) override;

signals:
	void newFrame(void* frame, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame);
	void maxFrames(int max);
	void maxBuffers(int max);
};

#endif // DEMOEXTENSION_H
