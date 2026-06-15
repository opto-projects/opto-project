#include "bitdepthconverter.h"
#include <QtMath>

#include <opencv2/opencv.hpp>

BitDepthConverter::BitDepthConverter(QObject *parent) : QObject(parent)
{
	this->output8bitData = nullptr;
	this->bitDepth = 0;
	this->length = 0;
	this->conversionRunning = false;
	outputColormap = nullptr;
}

BitDepthConverter::~BitDepthConverter()
{
	if(this->output8bitData != nullptr){
		free(this->output8bitData);
	}
	if (this->outputColormap != nullptr) {
		free(this->outputColormap);
	}
}

void BitDepthConverter::convertDataToColormap(QString colormap, void* inputData, int bitDepth, int width, int height, int channel)
{
	
	if (!this->conversionRunning) {
		this->conversionRunning = true;
		unsigned int bytesPerPixel = channel;
		int length = width * height;
		cv::Mat im_gray = cv::Mat(height, width, CV_8UC1, static_cast<uchar*>(inputData));
		cv::Mat im_colormap;

		if (!colormap.isEmpty())
		{
			bytesPerPixel = 3;

			if (this->outputColormap == nullptr || this->bitDepth != bitDepth || this->length != length) {
				if (bitDepth == 0 || length == 0 || channel != 1) {
					emit error(tr("BitDepthConverter: Invalid data dimensions!"));
					this->conversionRunning = false;
					return;
				}
				this->bitDepth = bitDepth;
				this->length = length;
				if (this->outputColormap != nullptr) {
					free(this->outputColormap);
					this->outputColormap = nullptr; //assign nullptr to avoid dangling pointer
				}
				this->outputColormap = static_cast<uchar*>(malloc(length * sizeof(uchar) * bytesPerPixel));
			}

			if (colormap.at(0).isDigit())
			{
				int idx = colormap.toInt();

				if (idx >=0 && idx<22)
				{
					cv::applyColorMap(im_gray, im_colormap, cv::ColormapTypes(idx));

					cv::cvtColor(im_colormap, im_colormap, cv::COLOR_BGR2RGB);

					memcpy(outputColormap, im_colormap.data, length * sizeof(uchar) * bytesPerPixel);
					emit convertedColormap(outputColormap, width, height, bytesPerPixel);

					this->conversionRunning = false;
					return;
				}
				
			}
			else
			{
				//todo load colormap
				/*cv::Mat lut;

				lut = cv::imread("", -1);*/
			}
		}
		emit convertedColormap(static_cast<uchar*>(inputData),  width, height, bytesPerPixel);

		this->conversionRunning = false;
	}
}

void BitDepthConverter::convertDataTo8bit(void *inputData, int bitDepth, int width, int height, int channel) {
	if(!this->conversionRunning){
		this->conversionRunning = true;
		int length = width * height;
		unsigned int bytesPerPixel = channel;
		//check if new output8bitData-buffer needs to be created (due to resize or first time use)
		if(this->output8bitData == nullptr || this->bitDepth != bitDepth || this->length != length){
			if(bitDepth == 0 || length == 0 || channel != 1){
				emit error(tr("BitDepthConverter: Invalid data dimensions!"));
				this->conversionRunning = false;
				return;
			}
			this->bitDepth = bitDepth;
			this->length = length;
			if(this->output8bitData != nullptr){
				free(this->output8bitData);
				this->output8bitData = nullptr; //assign nullptr to avoid dangling pointer
			}
			this->output8bitData = static_cast<uchar*>(malloc(length*sizeof(uchar)));
		}
		//no conversion needed if inputData is already 8bit or below
		if (bitDepth <= 8){
			for(int i=0; i<length; i++){
				this->output8bitData[i] = static_cast<ushort*>(inputData)[i]; //todo: replace this for loop by memcpy
			}
		}
		//convert to 8 bit element by element
		else if (bitDepth >= 9 && bitDepth <=16){
			float factor = 255 / (qPow(2,bitDepth) - 1);
			for(int i=0; i<length; i++){
				this->output8bitData[i] = static_cast<ushort*>(inputData)[i] * factor;
				//this->output8bitData[i] = static_cast<uchar*>(inputData)[2*i+1]; //for 16 bit to 8 bit this is also possible
			}
		}
		else if (bitDepth > 16 && bitDepth <=32){
			bytesPerPixel = 3;
			float factor = 255 / (qPow(2,bitDepth) - 1);
			for(int i=0; i<length; i++){
				this->output8bitData[i] = static_cast<unsigned int*>(inputData)[i] * factor;
			}
		//do nothing if bit depth is out of range
		}else{
			this->conversionRunning = false;
			return;
		}

		emit converted8bitData(output8bitData, bitDepth, width, height, bytesPerPixel);
		this->conversionRunning = false;
	}
}
