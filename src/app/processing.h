

#ifndef PROCESSING_H
#define PROCESSING_H

#include <QObject>
#include "polynomial.h"
#include "octproz_devkit.h"
#include "kernels.h"
#include "recorder.h"
#include "settings.h"
#include "octalgorithmparameters.h"
#include "gpu2hostnotifier.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QOffscreenSurface>
#include <QOpenGLContext>

#define LOW_FRAMERATE 12.5


class Processing : public QObject
{
	Q_OBJECT
	QThread recordingRawThread;
	QThread recordingProcessedThread;

public:
	Processing();
	~Processing();
	QOpenGLContext* context;
	QOffscreenSurface* surface;


private:
	void initCudaOpenGlInterop();
	bool waitForCudaOpenGlInteropReady(int interval, int timeout);
	bool isCudaOpenGlInteropReady();
	void blockBuffersForAcquisitionSystem(AcquisitionSystem* system);
	void unblockBuffersForAcquisitionSystem(AcquisitionSystem* system);

	bool bscanGlBufferRegisteredWithCuda;
	bool enfaceGlBufferRegisteredWithCuda;
	bool volumeGlBufferRegisteredWithCuda;
	qreal buffersPerSecond;
	bool isProcessing;
	OctAlgorithmParameters* octParams;
	bool recordingRawEnabled;
	Recorder* rawRecorder;
	Recorder* processedRecorder;
	AcquisitionBuffer* streamingBuffer;
	unsigned int currBufferNr;


public slots :
	//todo: decide if prefix "slot_" should be used or not and change naming of slots accordingly
	void slot_start(AcquisitionSystem* system);
	void slot_enableRecording(RecordingParams recParams);
	void slot_updateDisplayedBscanFrame(unsigned int frameNr, unsigned int displayFunctionFrames, int displayFunction);
	void slot_updateDisplayedEnFaceFrame(unsigned int frameNr, unsigned int displayFunctionFrames, int displayFunction);
#ifdef WITH_CUDA
	void slot_registerBscanOpenGLbufferWithCuda(unsigned int openGLbufferId);
	void slot_registerEnFaceViewOpenGLbufferWithCuda(unsigned int openGLbufferId);
	void slot_registerVolumeViewOpenGLbufferWithCuda(unsigned int openGLbufferId);
	void enableGpu2HostStreaming(bool enableStreaming);
	void registerStreamingHostBuffers(void* h_streamingBuffer1, void* h_streamingBuffer2, size_t bytesPerBuffer);
	void unregisterStreamingdHostBuffers();
#endif

signals :
	//void initOpenGL(QOpenGLContext** processingContext, QOffscreenSurface** processingSurface, QThread* processingThread);
	void initializationDone();
	void initializationFailed();
	void initOpenGL(QOpenGLContext* processingContext, QOffscreenSurface* processingSurface, QThread* processingThread);
	void initOpenGLenFaceView();
	void initRawRecorder(RecordingParams params);
	void initProcessedRecorder(RecordingParams params);
	void processingDone();
	void streamingBufferEnabled(bool enabled);

	void processedRecordDone();
	void rawRecordDone();
	void rawData(void* rawBuffer, unsigned bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame, unsigned int framesPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr);
	void info(QString info);
	void error(QString error);
	void updateInfoBox(QString volumesPerSecond, QString buffersPerSecond, QString bscansPerSecond, QString ascansPerSecond, QString bufferSizeMB, QString dataThroughput);
};

#endif // PROCESSING_H
