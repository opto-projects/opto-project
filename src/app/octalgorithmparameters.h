
#ifndef OCTALGORITHMPARAMETERS_H
#define OCTALGORITHMPARAMETERS_H

#include "polynomial.h"
#include "windowfunction.h"
#include <QString>

#define FIXED_PATTERN_NOISE_REMOVAL_SEGMENTS 9


enum INTERPOLATION {
	LINEAR,
	CUBIC,
	LANCZOS
};

struct RecordingParams {
	QString timestamp;
	QString fileName;
	QString savePath;
	size_t bufferSizeInBytes;
	unsigned int buffersToRecord;
	bool startWithFirstBuffer;
	bool recordRaw;
	bool recordProcessed;
	bool recordScreenshot;
	bool saveMetaData;
	bool stopAfterRecord;
};


class OctAlgorithmParameters
{
public:
	/**
	* Constructor
	* @note Singleton pattern
	*
	**/
	static OctAlgorithmParameters* getInstance();


	/**
	* Destructor
	*
	**/
	~OctAlgorithmParameters();

	void updateBufferSizeInBytes();
	void updateResampleCurve();
	void updateDispersionCurve();
	void updateWindowCurve();
	void updatePostProcessingBackgroundCurve();
	void loadCustomResampleCurve(float* externalCurve, int size);
	void loadPostProcessingBackground(float* background, int size);
	
	//acquisition
	unsigned int samplesPerLine;
	unsigned int ascansPerBscan;
	unsigned int bscansPerBuffer;
	unsigned int buffersPerVolume;
	unsigned int bitDepth;
	bool acquisitionParamsChanged;
	
	//processing
	bool bitshift;	/// Activating/Deactivating bit shift. This is needed if 12 bit values are transported as 2 bytes (= 16 bit) from the Alazar digitizer board ATS9373 for example
	bool bscanFlip; ///	Activating/Deactivating flipping of every second B-scan. This is needed if B-scans are acquired in forward and backward scan direction
	bool signalLogScaling; /// This variable is for activating/deactivating log scaling in OCT signal processing
	bool sinusoidalScanCorrection; /// Activating/Deactivating sinusoidal scan correction (corrects image distortion due to sinus scan of fast axis scanner)
	float signalGrayscaleMin; /// User defined minimal signal value. Values greater than this will be cut off.
	float signalGrayscaleMax; /// User defined maximal signal value. Values smaller than this will be cut off.
	float signalMultiplicator; /// User defined multiplicator. Each signal value will be multiplied with this value. 
	float signalAddend;	/// User defined addend. This value will be added to each signal value. 

	bool backgroundRemoval;
	int rollingAverageWindowSize;

	float* resampleCurve;
	float* customResampleCurve;
	float* resampleReferenceCurve;
	float c0, c1, c2, c3;
	int resampleCurveLength;
	int customResampleCurveLength;
	bool resampling;
	bool resamplingUpdated;
	bool useCustomResampleCurve;
	INTERPOLATION resamplingInterpolation;
	QString customResampleCurveFilePath;

	float* dispersionCurve;
	float* dispersionReferenceCurve;
	float d0, d1, d2, d3;
	bool dispersionCompensation;
	bool dispersionUpdated;

	float* windowCurve;
	float* windowReferenceCurve;
	WindowFunction::WindowType window;
	float windowCenter, windowFillFactor;
	bool windowing;
	bool windowUpdated;

	bool fixedPatternNoiseRemoval;
	bool continuousFixedPatternNoiseDetermination;
	bool redetermineFixedPatternNoise;
	unsigned int bscansForNoiseDetermination;
	
	//post processing
	bool postProcessBackgroundRemoval;
	bool postProcessBackgroundRecordingRequested;
	float postProcessBackgroundWeight;
	float postProcessBackgroundOffset;
	float* postProcessBackground;
	int postProcessBackgroundLength;
	bool postProcessBackgroundUpdated;

	//visualization
	//todo: put all visualization params in a single struct and use an enum for displayfunction
	unsigned int frameNr; /// Current number of the displayed frame.
	unsigned int frameNrEnFaceView;
	unsigned int functionFramesEnFaceView;
	unsigned int functionFramesBscan;
	int displayFunctionBscan;
	int displayFunctionEnFaceView;
	enum DISPLAY_FUNCTION {
		AVERAGING,
		MIP
	};
	bool bscanViewEnabled;
	bool enFaceViewEnabled;
	bool volumeViewEnabled;

	//recording
	RecordingParams recParams;


	//streaming
	bool streamingParamsChanged;
	bool streamToHost;
	unsigned int streamingBuffersToSkip;
	unsigned int currentBufferNr;


private:
	OctAlgorithmParameters(void);
	static OctAlgorithmParameters* octAlgorithmParameters;

	float* resizeCurve(float* curve, int currentSize, int newSize);

	Polynomial* resamplingCurveCalculator;
	Polynomial* resamplingReferenceCurveCalculator;
	Polynomial* dispersionCurveCalculator;
	Polynomial* dispersionReferenceCurveCalculator;
	WindowFunction* windowCurveCalculator;
	WindowFunction* windowReferenceCurveCalculator;
};


#endif // OCTALGORITHMPARAMETERS_H
