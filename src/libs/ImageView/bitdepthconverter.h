#ifndef BITDEPTHCONVERTER_H
#define BITDEPTHCONVERTER_H

#include <QObject>

namespace cv {
	class Mat;
}

class BitDepthConverter : public QObject
{
	Q_OBJECT
public:
	explicit BitDepthConverter(QObject *parent = nullptr);
	~BitDepthConverter();

private:
	uchar* output8bitData;
	int bitDepth;
	int length;
	bool conversionRunning;

	uchar* outputColormap;
public slots:
	void convertDataTo8bit(void *inputData, int bitDepth, int width, int height, int channel);
	void convertDataToColormap(QString colormap, void* inputData, int bitDepth, int width, int height, int channel);

signals:
	void converted8bitData(uchar *output8bitData, unsigned int bitDepth, unsigned int width, unsigned int height, unsigned int bytesPerPixel);
	void convertedColormap(uchar* outputData,  unsigned int width, unsigned int height, unsigned int bytesPerPixel);
	void info(QString);
	void error(QString);
};
#endif // BITDEPTHCONVERTER_H
